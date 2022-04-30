#include "net_utils.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "esp_log.h"
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

  server.begin();

  ESP_LOGI(TAG, "Web Server is up!");

  return server;
}

static void startWifiAp()
{
  ESP_LOGI(TAG, "Attempting to start Soft AP");

  const auto networkList = createNetworkList();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("sensor-1f2a46kg", "1f2a46kg");
  ESP_LOGI(TAG, "Soft AP is up!");

  const auto webServer = createWebServer(networkList);

  ESP_LOGI(TAG, "Web server is running");
}

void NetUtils::startWifi()
{
  log_i("Attempting to connect to WiFi");

  /* if (WiFi.begin() != WL_CONNECT_FAILED)
  {
    log_i("Connection successful");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(5000);
    }

    return;
  } */

  startWifiAp();
}