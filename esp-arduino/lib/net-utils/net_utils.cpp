#include <FreeRTOS.h>

#include "net_utils.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "cJSON.h"
#include "crypto_utils.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "spiffs_utils.h"
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>

#define KEY_SIZE 32

static constexpr auto *TAG = "NET";

static const String ATTESTATION_SERVER =
    "http://192.168.0.180:8080/attestation/";
static const String CLIENT_HELLO_ENDPOINT    = "clientHello";
static const String KEY_EXCHANGE_ENDPOINT    = "keyExchange";
static const String CLIENT_FINISHED_ENDPOINT = "clientFinished";

static String createNetworkList()
{
  ESP_LOGI(TAG, "Will now turn on STATION mode and commence network scan!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(3000);

  const auto noNetworks = WiFi.scanNetworks();
  ESP_LOGI(TAG, "Found %d networks", noNetworks);

  String result = "";

  for (auto i = 0; i < noNetworks; ++i)
  {
    result += "<option class=\"list-group-item\">";
    result += WiFi.SSID(i).c_str();
    result += "</option>";
  }

  return result;
}

static void startWifiSta(const String &ssid, const String &pass)
{
  const auto success = WiFi.softAPdisconnect();
  if (!success)
  {
    ESP_LOGE(TAG, "SoftAP couldn't be switched off");
  }

  wifi_config_t wifiConfig{};
  strcpy((char *)wifiConfig.sta.ssid, "WiFi-2.4");
  strcpy((char *)wifiConfig.sta.password, "180898Delia!");

  const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
  ESP_LOGI(TAG, "Credential set... Restarting");
  ESP.restart();
}

static AsyncWebServer createWebServer(const String &networkList)
{
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "Couldn't mount filesystem");
  }

  AsyncWebServer server(80);
  server.on("/", HTTP_GET,
            [&networkList](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /");
              request->send(SPIFFS, "/index.html", String(), false,
                            [&networkList](const String &var)
                            {
                              if (var == "NETWORKS")
                              {
                                return networkList;
                              }

                              return String();
                            });
            });
  server.on("/bootstrap.css", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /bootstrap.css");
              request->send(SPIFFS, "/bootstrap.css", "text/css");
            });
  server.on("/jquery.js", HTTP_GET,
            [](AsyncWebServerRequest *request)

            {
              ESP_LOGI(TAG, "Received GET request on /jquery.js");
              request->send(SPIFFS, "/jquery.js", "text/javascript");
            });
  server.on("/popper.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /popper.js");
              request->send(SPIFFS, "/popper.js", "text/javascript");
            });
  server.on("/bootstrap.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /bootstrap.js");
              request->send(SPIFFS, "/bootstrap.js", "text/javascript");
            });
  server.on(
      "/wifi", HTTP_POST,
      [](AsyncWebServerRequest *request)
      {
        // do nothing
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final)
      {
        // do nothing
      },
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total)
      {
        ESP_LOGI(TAG, "Received POST request on /wifi with body");
        request->send(200);
        ESP_LOGI(TAG, "Received data from website", data);

        const auto delimiter = " ";
        const auto ssid      = strtok((char *)data, delimiter);
        const auto pass      = strtok(nullptr, delimiter);

        startWifiSta(ssid, pass);
      });

  server.begin();

  ESP_LOGI(TAG, "Web Server is up!");

  return server;
}

static void startWifiAp()
{
  ESP_LOGI(
      TAG,
      "WiFi connection couldn't be established. Attempting to start Soft AP");

  const auto networkList = createNetworkList();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("sensor-1f2a46kg", "3599Andrei!");
  const auto apIp = WiFi.softAPIP().toString();
  ESP_LOGI(TAG, "Soft AP is up! IP: %s", apIp.c_str());

  const auto webServer = createWebServer(networkList);

  ESP_LOGI(TAG, "Web server is running");

  while (true)
  {
    delay(5000); // avoid destruction of the web server
                 // just wait for the credentials
  }
}

void NetUtils::startWifi()
{
  ESP_LOGI(TAG, "Attempting to connect to WiFi");

  if (WiFi.begin() == WL_CONNECT_FAILED)
  {
    startWifiAp();
  }

  ESP_LOGI(TAG, "Credentials are ok... waiting for WiFi to go up");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
  }

  ESP_LOGI(TAG, "Connection successful");
}

static bool checkExistingKey(const std::string &content)
{
  /* TODO add signature check
   format:
   key(hex) timestamp
   signature
   */

  if (content.empty())
  {
    return false;
  }
}

static std::unique_ptr<uint8_t[]> performClientHello()
{
  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_HELLO_ENDPOINT;
  client.begin(endpoint);

  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  const auto statusCode = client.POST(
      "LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNKekNDQWM2Z0F3SUJBZ0lVRWV0TlN4"
      "cEEvanY0NlhYYnpVbmwySzJwMDYwd0NnWUlLb1pJemowRUF3SXcKYVRFTE1Ba0dBMVVFQmhN"
      "Q1VrOHhFREFPQmdOVkJBZ01CMUp2YldGdWFXRXhFakFRQmdOVkJBY01DVUoxWTJoaApjbVZ6"
      "ZERFS01BZ0dBMVVFQ2d3Qkx6RUtNQWdHQTFVRUN3d0JMekVLTUFnR0ExVUVBd3dCTHpFUU1B"
      "NEdDU3FHClNJYjNEUUVKQVJZQkx6QWVGdzB5TWpBME16QXhPRFV4TURCYUZ3MHlNekEwTWpV"
      "eE9EVXhNREJhTUdreEN6QUoKQmdOVkJBWVRBbEpQTVJBd0RnWURWUVFJREFkU2IyMWhibWxo"
      "TVJJd0VBWURWUVFIREFsQ2RXTm9ZWEpsYzNReApDakFJQmdOVkJBb01BUzh4Q2pBSUJnTlZC"
      "QXNNQVM4eENqQUlCZ05WQkFNTUFTOHhFREFPQmdrcWhraUc5dzBCCkNRRVdBUzh3V2pBVUJn"
      "Y3Foa2pPUFFJQkJna3JKQU1EQWdnQkFRY0RRZ0FFUzdEclF4TkRNeFVmeDdGL1NkZUQKSDFB"
      "OW50dEswbHJTRS92RHlOYW9DNFlnekM4MngvVy80cWdYL1NobVUzVnhvKzhobjd3czNxZ0RZ"
      "Z3d1S2lOeQo0S05UTUZFd0hRWURWUjBPQkJZRUZBVXJZaUs0S3NtMEpBbDlNR3pBZk9uQWM2"
      "SnRNQjhHQTFVZEl3UVlNQmFBCkZBVXJZaUs0S3NtMEpBbDlNR3pBZk9uQWM2SnRNQThHQTFV"
      "ZEV3RUIvd1FGTUFNQkFmOHdDZ1lJS29aSXpqMEUKQXdJRFJ3QXdSQUlnTEFwRG5wTHp1by9n"
      "REpwVXEyM1NvRVVrSndSd0Vnb0xWZGRWMnd1ekdjVUNJQ3hnSzBRWgpEMDZ3UDAvZkVTSzdR"
      "cU1jT3dvK1hUOXVhVlVhSW9YNWdEblUKLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQ==");

  ESP_LOGI(TAG, "Server responded %d", statusCode);
  size_t length = 0;
  auto   serverCertificate =
      crypto::decodeBase64((uint8_t *)client.getString().c_str(), length);

  ESP_LOGI(TAG, "Server certificate has been decoded");

  return std::move(serverCertificate);
}

static String performKeyExchange()
{
  HTTPClient client;

  const auto ecdhParams = crypto::generateEcdhParams();

  ESP_LOGI(TAG, "DH params have been generated");
  uint8_t buffer[KEY_SIZE * 2 + 1];
  buffer[0] = 0x04;
  mbedtls_mpi_write_binary(&ecdhParams.Q.X, buffer + 1, KEY_SIZE);
  mbedtls_mpi_write_binary(&ecdhParams.Q.Y, buffer + KEY_SIZE + 1, KEY_SIZE);

  size_t     outputLength = 0;
  const auto encodedParams =
      crypto::encodeBase64(buffer, KEY_SIZE * 2 + 1, outputLength);
  ESP_LOGI(TAG, "Encoded DH params: %s", encodedParams.c_str());

  uint8_t privateKey[] = {
      0x30, 0x78, 0x02, 0x01, 0x01, 0x04, 0x20, 0xa0, 0xc4, 0x6b, 0x41, 0x54,
      0xa0, 0x14, 0x06, 0xe9, 0xff, 0xc3, 0x46, 0x95, 0x89, 0x4c, 0xca, 0x52,
      0x18, 0xcc, 0xd1, 0xd4, 0x7f, 0x53, 0x55, 0xc6, 0xe7, 0x12, 0x91, 0x0a,
      0xc0, 0xbd, 0x79, 0xa0, 0x0b, 0x06, 0x09, 0x2b, 0x24, 0x03, 0x03, 0x02,
      0x08, 0x01, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x4b, 0xb0,
      0xeb, 0x43, 0x13, 0x43, 0x33, 0x15, 0x1f, 0xc7, 0xb1, 0x7f, 0x49, 0xd7,
      0x83, 0x1f, 0x50, 0x3d, 0x9e, 0xdb, 0x4a, 0xd2, 0x5a, 0xd2, 0x13, 0xfb,
      0xc3, 0xc8, 0xd6, 0xa8, 0x0b, 0x86, 0x20, 0xcc, 0x2f, 0x36, 0xc7, 0xf5,
      0xbf, 0xe2, 0xa8, 0x17, 0xfd, 0x28, 0x66, 0x53, 0x75, 0x71, 0xa3, 0xef,
      0x21, 0x9f, 0xbc, 0x2c, 0xde, 0xa8, 0x03, 0x62, 0x0c, 0x2e, 0x2a, 0x23,
      0x72, 0xe0};
  size_t     signatureLength;
  const auto signature = crypto::signEcdsa(buffer, KEY_SIZE * 2 + 1,
                                           signatureLength, privateKey, 122);

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

static void performClientFinish(const char *publicParams, const char *signature,
                                const char    *test,
                                const uint8_t *serverCertficate)
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

  uint8_t ptX[32] = {0x9E, 0x96, 0xE8, 0xCE, 0x84, 0xD3, 0x55, 0x13,
                     0x53, 0x52, 0xFA, 0x38, 0x1A, 0xC0, 0xFD, 0xC,
                     0x8B, 0xCE, 0xDE, 0x40, 0x9F, 0x6A, 0x18, 0x85,
                     0xF,  0x3E, 0xE4, 0xF5, 0xEF, 0x4,  0x37, 0x96};
  mbedtls_mpi_read_binary(&point.X, ptX, 32);

  uint8_t ptY[32] = {0x30, 0xDA, 0xAE, 0xB,  0xD9, 0xD8, 0x2A, 0x84,
                     0xC1, 0x13, 0xAE, 0xFD, 0xEA, 0x1D, 0x15, 0x6B,
                     0x93, 0x5C, 0x57, 0xE0, 0xA8, 0xD2, 0xF3, 0xB4,
                     0xD3, 0xDD, 0xCD, 0x24, 0xAA, 0x49, 0x28, 0x17};
  mbedtls_mpi_read_binary(&point.Y, ptY, 32);

  mbedtls_mpi_lset(&point.Z, 1);

  crypto::verifyEcdsa(decodedPublicParams.get(), paramsLength,
                      decodedSignature.get(), signatureLength, point);
}

static void performAttestationProcess()
{
  ESP_LOGI(TAG, "Starting the attestation process");

  const auto serverCertificate = performClientHello();
  const auto serverPayload     = performKeyExchange();

  auto       *root = cJSON_Parse(serverPayload.c_str());
  const auto *publicParams =
      cJSON_GetObjectItem(root, "publicParams")->valuestring;
  const auto *signature = cJSON_GetObjectItem(root, "signature")->valuestring;
  const auto *test      = cJSON_GetObjectItem(root, "test")->valuestring;

  performClientFinish(publicParams, signature, test, serverCertificate.get());
}

static bool checkAttestationStatus()
{
  auto       spiffs  = SpiffsUtils::getInstance();
  const auto content = spiffs->readText("/secret.key");

  return checkExistingKey(content);
}

void NetUtils::attestDevice()
{
  // TODO add rtos task to check key each hour

  ESP_LOGI(TAG, "Checking attestation status");

  if (checkAttestationStatus())
  {
    return;
  }

  performAttestationProcess();
}