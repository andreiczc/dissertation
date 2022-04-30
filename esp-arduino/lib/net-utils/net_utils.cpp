#include <FreeRTOS.h>

#include "net_utils.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

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

static void performAttestationProcess()
{
  ESP_LOGI(TAG, "Starting the attestation process");

  HTTPClient client;
  auto       endpoint = ATTESTATION_SERVER + CLIENT_HELLO_ENDPOINT;
  client.begin(endpoint);

  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  auto statusCode = client.POST(
      "LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNJekNDQWNxZ0F3SUJBZ0lVTFA2Yjhm"
      "VGM0ZGxpMHlMZGM0c3pjNVZGWWlVd0NnWUlLb1pJemowRUF3SXcKWnpFTE1Ba0dBMVVFQmhN"
      "Q1VrOHhFREFPQmdOVkJBZ01CMUp2YldGdWFXRXhFakFRQmdOVkJBY01DVUoxWTJoaApjbVZ6"
      "ZERFaE1COEdBMVVFQ2d3WVNXNTBaWEp1WlhRZ1YybGtaMmwwY3lCUWRIa2dUSFJrTVE4d0RR"
      "WURWUVFECkRBWmtkMkZrZDJFd0hoY05Nakl3TXpJMU1Ea3lPREkyV2hjTk1qTXdNekl3TURr"
      "eU9ESTJXakJuTVFzd0NRWUQKVlFRR0V3SlNUekVRTUE0R0ExVUVDQXdIVW05dFlXNXBZVEVT"
      "TUJBR0ExVUVCd3dKUW5WamFHRnlaWE4wTVNFdwpId1lEVlFRS0RCaEpiblJsY201bGRDQlhh"
      "V1JuYVhSeklGQjBlU0JNZEdReER6QU5CZ05WQkFNTUJtUjNZV1IzCllUQmFNQlFHQnlxR1NN"
      "NDlBZ0VHQ1Nza0F3TUNDQUVCQndOQ0FBUUtvTTlnVXdYbGdGa2EvL2o0N2p5eTdNeTQKWjRC"
      "YVdMbmdZSmkzdVVtZ2ZGangxc1l2blhrQXcyVHN0MDFuTTJLMGIrU2laZTVJU3h2RmxmMkJv"
      "dkZPbzFNdwpVVEFkQmdOVkhRNEVGZ1FVWCsyTFNtOFNjeEllZExKMmVlaGJlUWNHMXg4d0h3"
      "WURWUjBqQkJnd0ZvQVVYKzJMClNtOFNjeEllZExKMmVlaGJlUWNHMXg4d0R3WURWUjBUQVFI"
      "L0JBVXdBd0VCL3pBS0JnZ3Foa2pPUFFRREFnTkgKQURCRUFpQWtOK1BBdVpDTWxJWFhFWlZt"
      "a1JPMHN2RHdONHJvVUVXSXNDVGFnTFBqa1FJZ1hEd0M2NHZUR1N6UwpWdFRPN0VJK0tVZVJp"
      "dkRSOFdNendQWXFOUXRpZ3JZPQotLS0tLUVORCBDRVJUSUZJQ0FURS0tLS0tCg==");

  ESP_LOGI(TAG, "Server responded %d", statusCode);
  const auto serverCertificate =
      crypto::decodeBase64((uint8_t *)client.getString().c_str());

  ESP_LOGI(TAG, "Server certificate has been decoded");
  const auto ecdhParams = crypto::generateEcdhParams();

  ESP_LOGI(TAG, "DH params have been generated");
  uint8_t buffer[KEY_SIZE * 2 + 1];
  buffer[0] = 0x04;
  mbedtls_mpi_write_binary(&ecdhParams.Q.X, buffer, KEY_SIZE);
  mbedtls_mpi_write_binary(&ecdhParams.Q.Y, buffer + KEY_SIZE, KEY_SIZE);

  const auto encodedParams = crypto::encodeBase64(buffer);
  ESP_LOGI(TAG, "Encoded DH params: %s", encodedParams.c_str());

  uint8_t privateKey[] = {
      0x30, 0x78, 0x02, 0x01, 0x01, 0x04, 0x20, 0x55, 0x49, 0x96, 0x60, 0xc5,
      0x9b, 0x1a, 0xa4, 0xfa, 0x77, 0xdc, 0xe3, 0x70, 0xc3, 0xfa, 0xf5, 0x11,
      0xd4, 0x66, 0x63, 0x71, 0xf5, 0xf3, 0x1c, 0x4a, 0x19, 0x88, 0x20, 0xfa,
      0x04, 0x8f, 0x01, 0xa0, 0x0b, 0x06, 0x09, 0x2b, 0x24, 0x03, 0x03, 0x02,
      0x08, 0x01, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x86, 0x0d,
      0x56, 0x42, 0x66, 0xdf, 0xe6, 0x94, 0xb3, 0xa8, 0xab, 0x8b, 0x68, 0xe2,
      0xa3, 0xe4, 0xd8, 0x2a, 0xfd, 0xa1, 0xb0, 0x2e, 0xe3, 0xa8, 0xe8, 0x95,
      0x25, 0xb9, 0x81, 0xb4, 0xb0, 0x6c, 0x45, 0x5e, 0xe8, 0x1c, 0x19, 0xf7,
      0x56, 0xd4, 0xa9, 0xd5, 0x75, 0xaa, 0xc6, 0x7e, 0xda, 0xb4, 0xcd, 0x52,
      0x16, 0x42, 0x39, 0x59, 0x94, 0xe8, 0x81, 0xc6, 0xb8, 0xbb, 0x11, 0xff,
      0xa9, 0x4a};
  size_t     signatureLength;
  const auto signature = crypto::signEcdsa(buffer, KEY_SIZE * 2 + 1,
                                           signatureLength, privateKey, 122);

  const auto signatureEncoded = crypto::encodeBase64(signature.get());

  String payload = "{\"publicParams\":" + encodedParams +
                   ",\"signature\":" + signatureEncoded.c_str() + "}";

  endpoint = ATTESTATION_SERVER + KEY_EXCHANGE_ENDPOINT;
  client.begin(endpoint);

  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  statusCode = client.POST(payload.c_str());
  ESP_LOGI(TAG, "Server responded %d", statusCode);
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