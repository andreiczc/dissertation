#include <FreeRTOS.h>

#include "net_utils.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "spiffs_utils.h"
#include <SPIFFS.h>
#include <WiFi.h>

static constexpr auto *TAG = "NET";

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

static void performAttestationProcess() {}

static bool checkAttestationStatus()
{
  auto       spiffs  = SpiffsUtils::getInstance();
  const auto content = spiffs->readText("/secret.key");

  return checkExistingKey(content);
}

void attestDevice()
{
  // TODO add rtos task to check key each hour

  ESP_LOGI(TAG, "Checking attestation status");

  if (checkAttestationStatus())
  {
    return;
  }

  performAttestationProcess();
}