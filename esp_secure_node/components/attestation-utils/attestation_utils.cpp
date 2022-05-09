#include "attestation_utils.h"

#include "esp_log.h"
#include "spiffs_utils.h"
#include <HTTPClient.h>

#define KEY_SIZE 32

static constexpr auto *TAG = "ATTESTATION";
static const String    ATTESTATION_SERVER =
    "http://130.162.253.10:8080/attestation/";
static const String DEVICE_CERT_PATH         = "/device.crt";
static const String DEVICE_KEY_PATH          = "/device.key";
static const String CLIENT_HELLO_ENDPOINT    = "clientHello";
static const String KEY_EXCHANGE_ENDPOINT    = "keyExchange";
static const String CLIENT_FINISHED_ENDPOINT = "clientFinished";

namespace attestation
{
bool checkExistingKey(const String &content)
{
  /* TODO add signature check
   format:
   key(hex) timestamp
   signature
   */

  if (content.isEmpty())
  {
    return false;
  }

  return false;
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
  auto   serverPoint =
      crypto::decodeBase64((uint8_t *)client.getString().c_str(), length);

  ESP_LOGI(TAG, "Server certificate has been decoded");

  return std::move(serverPoint);
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

std::unique_ptr<uint8_t[]> performClientFinish(const char    *publicParams,
                                               const char    *signature,
                                               const char    *test,
                                               const uint8_t *serverPoint,
                                               mbedtls_ecdh_context &ecdhParams)
{
  size_t     paramsLength = 0;
  const auto decodedPublicParams =
      crypto::decodeBase64((uint8_t *)publicParams, paramsLength);

  size_t     signatureLength = 0;
  const auto decodedSignature =
      crypto::decodeBase64((uint8_t *)signature, signatureLength);

  mbedtls_ecp_point point;
  mbedtls_ecp_point_init(&point);

  mbedtls_mpi_init(&point.X);
  mbedtls_mpi_init(&point.Y);
  mbedtls_mpi_init(&point.Z);

  uint8_t pt[KEY_SIZE] = "";
  memcpy(pt, serverPoint, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.X, pt, KEY_SIZE);

  memcpy(pt, serverPoint + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, pt, KEY_SIZE);

  mbedtls_mpi_lset(&point.Z, 1);

  auto signatureVerifies =
      crypto::verifyEcdsa(decodedPublicParams.get(), paramsLength,
                          decodedSignature.get(), signatureLength, point);

  if (!signatureVerifies)
  {
    ESP_LOGE(TAG, "Attestation server might be compromised");
    ESP.restart();
  }

  memcpy(pt, decodedPublicParams.get() + 1, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.X, pt, KEY_SIZE);

  memcpy(pt, decodedPublicParams.get() + 1 + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, pt, KEY_SIZE);

  mbedtls_mpi_lset(&point.Z, 1);

  auto generatedSecret = crypto::generateSharedSecret(ecdhParams, point);

  ESP_LOGI(TAG, "Test bytes: %s", test);

  size_t     testLength = 0;
  const auto testBytes  = crypto::decodeBase64((uint8_t *)test, testLength);

  const auto iv       = crypto::generateRandomSequence(KEY_SIZE / 2);
  size_t     ivLength = 0;
  const auto ivBase64 = crypto::encodeBase64(iv.get(), KEY_SIZE / 2, ivLength);

  ESP_LOGI(TAG, "IV: %s", ivBase64.c_str());

  uint8_t payload[KEY_SIZE] = "";
  memcpy(payload, iv.get(),
         KEY_SIZE / 2); // before encryption since the IV is mutated!

  size_t     outputLength2 = 0;
  const auto keyEncoded =
      crypto::encodeBase64(generatedSecret.get(), 32, outputLength2);
  ESP_LOGI(TAG, "Key: %s", keyEncoded.c_str());

  uint16_t   cipherTextSize = 0;
  const auto cipherText     = crypto::encryptAes(
          testBytes.get(), generatedSecret.get(), iv.get(), cipherTextSize);

  memcpy(payload + KEY_SIZE / 2, cipherText.get(), KEY_SIZE / 2);

  size_t     outputLength = 0;
  const auto payloadEncoded =
      crypto::encodeBase64(payload, KEY_SIZE, outputLength);

  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_FINISHED_ENDPOINT;
  client.begin(endpoint);
  client.addHeader("Content-Type", "text/plain");
  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  ESP_LOGI(TAG, "Post data: %s", payloadEncoded.c_str());

  const auto statusCode = client.POST(payloadEncoded);
  ESP_LOGI(TAG, "Server responded %d", statusCode);

  return std::move(generatedSecret);
}
} // namespace attestation