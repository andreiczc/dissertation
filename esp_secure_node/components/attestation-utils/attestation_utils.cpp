#include "attestation_utils.h"

#include "cJSON.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "spiffs_utils.h"
#include <HTTPClient.h>
#include <map>
#include <vector>

#define KEY_SIZE   32
#define BLOCK_SIZE 16
#define MAC_SIZE   6

static const std::map<int, std::vector<int>> OBJECT_IDS{
    {1001, {2001}},
    {1002, {2002}},
    {1003, {2003}},
    {1004, {2004}},
};
static constexpr auto *TAG             = "ATTESTATION";
static constexpr auto *SECRET_KEY_PATH = "/secret.key";
static constexpr auto  UNIX_DAY        = 86400;
static const String    ATTESTATION_SERVER =
    "http://193.122.11.148:8082/attestation/";
static const String DEVICE_CERT_PATH         = "/device.crt";
static const String DEVICE_CERT_DER_PATH     = "/device.crt.der";
static const String DEVICE_KEY_PATH          = "/device.key";
static const String CLIENT_HELLO_ENDPOINT    = "clientHello";
static const String KEY_EXCHANGE_ENDPOINT    = "keyExchange";
static const String CLIENT_FINISHED_ENDPOINT = "clientFinished";

namespace attestation
{

static unsigned long getCurrentTime()
{
  tm timeInfo{};
  getLocalTime(&timeInfo);

  time_t now{};
  time(&now);

  return now;
}

static mbedtls_ecp_point deserializePoint(const uint8_t *publicPoint)
{
  mbedtls_ecp_point point;
  mbedtls_ecp_point_init(&point);

  mbedtls_mpi_init(&point.X);
  mbedtls_mpi_init(&point.Y);
  mbedtls_mpi_init(&point.Z);

  mbedtls_mpi_read_binary(&point.X, publicPoint, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, publicPoint + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_lset(&point.Z, 1);

  return point;
}

static void persistKey(const uint8_t *key, size_t keyLength, int instanceId)
{
  ESP_LOGI(TAG, "Persisting attestation information");

  const auto keyHex           = crypto::toHex(key, keyLength);
  const auto timestamp        = String(getCurrentTime());
  const auto instanceIdString = String(instanceId);

  const auto bufferSize =
      keyHex.length() + instanceIdString.length() + timestamp.length();
  std::unique_ptr<uint8_t> buffer(new uint8_t[bufferSize]);

  memcpy(buffer.get(), keyHex.c_str(), keyHex.length());
  memcpy(buffer.get() + keyHex.length(), instanceIdString.c_str(),
         instanceIdString.length());
  memcpy(buffer.get() + instanceIdString.length() + keyHex.length(),
         timestamp.c_str(), timestamp.length());

  ESP_LOG_BUFFER_HEX(TAG, buffer.get(), bufferSize);

  const auto privateKeyBase64 =
      SpiffsUtils::getInstance()->readText(DEVICE_KEY_PATH);
  size_t     privateKeyLength = 0;
  const auto privateKey       = crypto::decodeBase64(
            (uint8_t *)privateKeyBase64.c_str(), privateKeyLength);

  size_t     signatureLength = 0;
  const auto signature =
      crypto::signEcdsa(buffer.get(), bufferSize, signatureLength,
                        privateKey.get(), privateKeyLength);

  const auto signatureHex = crypto::toHex(signature.get(), signatureLength);

  const auto result =
      keyHex + " " + instanceIdString + " " + timestamp + " " + signatureHex;

  ESP_LOGI(TAG, "Persisting attestation info: %s", result.c_str());

  SpiffsUtils::getInstance()->writeText(SECRET_KEY_PATH, result);
}

bool checkExistingKey()
{
  /*
   format:
   key(hex) instanceId timestamp signature(hex)
   */

  auto spiffs  = SpiffsUtils::getInstance();
  auto content = spiffs->readText(SECRET_KEY_PATH);

  ESP_LOGI(TAG, "Verifying information about the stored key: %s",
           content.c_str());

  if (content.isEmpty())
  {
    ESP_LOGI(TAG, "No existing key content...");

    return false;
  }

  const auto keyHex       = content.substring(0, content.indexOf(" "));
  content                 = content.substring(content.indexOf(" ") + 1);
  const auto instanceId   = content.substring(0, content.indexOf(" "));
  content                 = content.substring(content.indexOf(" ") + 1);
  const auto timestamp    = content.substring(0, content.indexOf(" "));
  const auto signatureHex = content.substring(content.indexOf(" ") + 1);

  ESP_LOGI(TAG, "Last attestation date: %s", timestamp.c_str());
  const auto time        = timestamp.toInt();
  const auto currentTime = getCurrentTime();
  if (currentTime - time > UNIX_DAY)
  {
    ESP_LOGI(TAG,
             "Attestation happened more than 1 day ago... Current time: %ld; "
             "Attestation time: %ld",
             currentTime, time);

    return false;
  }

  size_t     signatureSize = 0;
  const auto signature     = crypto::fromHex(signatureHex, signatureSize);
  ESP_LOGI(TAG, "Deserialized signature:");
  ESP_LOG_BUFFER_HEX(TAG, signature.get(), signatureSize);

  const auto bufferSize =
      keyHex.length() + instanceId.length() + timestamp.length();
  std::unique_ptr<uint8_t> buffer(new uint8_t[bufferSize]);
  memcpy(buffer.get(), keyHex.c_str(), keyHex.length());
  memcpy(buffer.get() + keyHex.length(), instanceId.c_str(),
         instanceId.length());
  memcpy(buffer.get() + keyHex.length() + instanceId.length(),
         timestamp.c_str(), timestamp.length());

  ESP_LOGI(TAG, "Buffer to be used for signature verification:");
  ESP_LOG_BUFFER_HEX(TAG, buffer.get(), bufferSize);

  const auto ownCertificateBase64 =
      SpiffsUtils::getInstance()->readText(DEVICE_CERT_DER_PATH);
  size_t     certificateLength = 0;
  const auto ownCertificate    = crypto::decodeBase64(
         (uint8_t *)ownCertificateBase64.c_str(), certificateLength);
  const auto publicPoint =
      crypto::parseCertificate(ownCertificate.get(), certificateLength);

  mbedtls_ecp_point point = deserializePoint(publicPoint.get());

  const auto signatureVerifies = crypto::verifyEcdsa(
      buffer.get(), bufferSize, signature.get(), signatureSize, point);
  ESP_LOGI(TAG, "Signature %s",
           signatureVerifies ? "verifies" : "doesn't verify");

  return signatureVerifies;
}

std::unique_ptr<uint8_t[]> extractExistingInfo(int &instanceId)
{
  ESP_LOGI(TAG, "Retrieving stored key");

  auto spiffs  = SpiffsUtils::getInstance();
  auto content = spiffs->readText(SECRET_KEY_PATH);

  if (content.isEmpty())
  {
    ESP_LOGI(TAG, "No existing key content...");
    ESP.restart();
  }

  const auto keyHex = content.substring(0, content.indexOf(" "));
  content           = content.substring(content.indexOf(" ") + 1);
  instanceId        = content.substring(0, content.indexOf(" ")).toInt();

  size_t keySize = 0;

  return crypto::fromHex(keyHex, keySize);
}

std::unique_ptr<uint8_t[]> performClientHello()
{
  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_HELLO_ENDPOINT;
  client.begin(endpoint);

  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  const auto deviceCertificate =
      SpiffsUtils::getInstance()->readText(DEVICE_CERT_PATH);
  const auto statusCode = client.POST(deviceCertificate.c_str());

  ESP_LOGI(TAG, "Server responded %d", statusCode);
  size_t length = 0;
  auto   serverCertificate =
      crypto::decodeBase64((uint8_t *)client.getString().c_str(), length);

  ESP_LOGI(TAG, "Server certificate has been decoded");

  if (!crypto::verifyCertificate(serverCertificate.get(), length))
  {
    ESP_LOGE(TAG,
             "Received certificate is not issued by known CA. Will restart...");
    ESP.restart();
  }

  return crypto::parseCertificate(serverCertificate.get(), length);
}

String performKeyExchange(mbedtls_ecdh_context &ecdhParams)
{
  HTTPClient client;

  ecdhParams = crypto::generateEcdhParams();

  ESP_LOGI(TAG, "DH params have been generated");
  uint8_t buffer[KEY_SIZE * 2 + 1];
  buffer[0] = 0x04;
  mbedtls_mpi_write_binary(&ecdhParams.Q.X, buffer + 1, KEY_SIZE);
  mbedtls_mpi_write_binary(&ecdhParams.Q.Y, buffer + KEY_SIZE + 1, KEY_SIZE);

  size_t     outputLength = 0;
  const auto encodedParams =
      crypto::encodeBase64(buffer, KEY_SIZE * 2 + 1, outputLength);
  ESP_LOGI(TAG, "Encoded DH params: %s", encodedParams.c_str());

  const auto privateKeyBase64 =
      SpiffsUtils::getInstance()->readText(DEVICE_KEY_PATH);
  size_t     keyLength = 0;
  const auto privateKey =
      crypto::decodeBase64((uint8_t *)privateKeyBase64.c_str(), keyLength);
  size_t     signatureLength;
  const auto signature = crypto::signEcdsa(
      buffer, KEY_SIZE * 2 + 1, signatureLength, privateKey.get(), 122);

  const auto signatureEncoded =
      crypto::encodeBase64(signature.get(), signatureLength, outputLength);

  String payload = "{\"publicParams\":\"" + encodedParams +
                   "\",\"signature\":\"" + signatureEncoded.c_str() + "\"}";
  ESP_LOGI(TAG, "Sending payload: %s", payload.c_str());

  const auto endpoint = ATTESTATION_SERVER + KEY_EXCHANGE_ENDPOINT;
  client.begin(endpoint);
  client.addHeader("Content-Type", "application/json");
  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());

  const auto statusCode = client.POST(payload.c_str());
  ESP_LOGI(TAG, "Server responded %d", statusCode);

  return client.getString();
}

static char *constructIdentifier()
{
  ESP_LOGI(TAG, "Constructing Node identifier");

  auto *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "macAddress", WiFi.macAddress().c_str());
  auto *array = cJSON_AddArrayToObject(json, "objects");

  for (const auto &item : OBJECT_IDS)
  {
    auto *curr = cJSON_CreateObject();
    cJSON_AddNumberToObject(curr, "objectId", item.first);

    auto *rscArray = cJSON_AddArrayToObject(curr, "resources");
    for (const auto &rsc : item.second)
    {
      auto *obj = cJSON_CreateObject();
      cJSON_AddNumberToObject(obj, "resourceId", rsc);
      cJSON_AddItemToArray(rscArray, obj);
    }

    cJSON_AddItemToArray(array, curr);
  }

  const auto string = cJSON_Print(json);
  cJSON_Delete(json);

  ESP_LOGI(TAG, "Identifier: %s", string);

  return string;
}

static int parseResponseForInstanceId(const String &text)
{
  ESP_LOGI(TAG, "Parsing the server response for Instance ID");
  auto       *root       = cJSON_Parse(text.c_str());
  const auto *objectList = cJSON_GetObjectItem(root, "objects");
  const auto *firstItem  = cJSON_GetArrayItem(objectList, 0);
  const auto  instanceId =
      cJSON_GetObjectItem(firstItem, "instanceId")->valueint;

  cJSON_Delete(root);

  return instanceId;
}

std::unique_ptr<uint8_t[]>
performClientFinish(const char *publicParams, const char *signature,
                    const char *test, const uint8_t *serverPoint,
                    mbedtls_ecdh_context &ecdhParams, int &instanceId)
{
  size_t     paramsLength = 0;
  const auto decodedPublicParams =
      crypto::decodeBase64((uint8_t *)publicParams, paramsLength);

  size_t     signatureLength = 0;
  const auto decodedSignature =
      crypto::decodeBase64((uint8_t *)signature, signatureLength);

  mbedtls_ecp_point point = deserializePoint(serverPoint);
  auto              signatureVerifies =
      crypto::verifyEcdsa(decodedPublicParams.get(), paramsLength,
                          decodedSignature.get(), signatureLength, point);

  if (!signatureVerifies)
  {
    ESP_LOGE(TAG, "Attestation server might be compromised");
    ESP.restart();
  }

  uint8_t pt[KEY_SIZE] = "";
  memcpy(pt, decodedPublicParams.get() + 1, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.X, pt, KEY_SIZE);

  memcpy(pt, decodedPublicParams.get() + 1 + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, pt, KEY_SIZE);

  mbedtls_mpi_lset(&point.Z, 1);

  auto generatedSecret = crypto::generateSharedSecret(ecdhParams, point);

  ESP_LOGI(TAG, "Test bytes: %s", test);

  size_t     testLength = 0;
  const auto testBytes  = crypto::decodeBase64((uint8_t *)test, testLength);

  const auto iv       = crypto::generateRandomSequence(BLOCK_SIZE);
  size_t     ivLength = 0;
  const auto ivBase64 = crypto::encodeBase64(iv.get(), BLOCK_SIZE, ivLength);

  ESP_LOGI(TAG, "IV: %s", ivBase64.c_str());

  const auto identifier   = constructIdentifier();
  const auto requiredSize = strlen(identifier);
  const auto paddedSize =
      requiredSize + (BLOCK_SIZE - requiredSize % BLOCK_SIZE);
  const auto totalSize = paddedSize + BLOCK_SIZE;

  std::unique_ptr<uint8_t[]> payload(new uint8_t[totalSize + BLOCK_SIZE]);
  memcpy(payload.get(), iv.get(),
         BLOCK_SIZE); // before encryption since the IV is mutated!

  std::unique_ptr<uint8_t[]> plaintext(new uint8_t[totalSize]);
  memset(plaintext.get(), 0, totalSize);
  memcpy(plaintext.get(), testBytes.get(), BLOCK_SIZE);
  memcpy(plaintext.get() + BLOCK_SIZE, identifier, requiredSize);

  size_t     outputLength2 = 0;
  const auto keyEncoded =
      crypto::encodeBase64(generatedSecret.get(), KEY_SIZE, outputLength2);
  ESP_LOGI(TAG, "Key: %s", keyEncoded.c_str());

  uint16_t   cipherTextSize = 0;
  const auto cipherText =
      crypto::encryptAes(plaintext.get(), totalSize, generatedSecret.get(),
                         iv.get(), cipherTextSize);

  memcpy(payload.get() + BLOCK_SIZE, cipherText.get(), totalSize);

  size_t     outputLength = 0;
  const auto payloadEncoded =
      crypto::encodeBase64(payload.get(), totalSize + BLOCK_SIZE, outputLength);

  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_FINISHED_ENDPOINT;
  client.begin(endpoint);
  client.addHeader("Content-Type", "text/plain");
  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  ESP_LOGI(TAG, "Post data: %s", payloadEncoded.c_str());

  const auto statusCode = client.POST(payloadEncoded);
  ESP_LOGI(TAG, "Server responded %d", statusCode);

  const auto responseText = client.getString();
  instanceId              = parseResponseForInstanceId(responseText);
  ESP_LOGI(TAG, "Instance ID is %d", instanceId);

  persistKey(generatedSecret.get(), KEY_SIZE, instanceId);

  return std::move(generatedSecret);
}
} // namespace attestation