#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

#include "logger.h"

static Logger         logger(Serial);
static AsyncWebServer server(80);

static void startWifi(bool in_memory = false)
{
  /* if (!in_memory)
  {
    read_wifi_credentials();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    logger.info("WiFi connection established!");
    empty_wifi_credentials();
    return;
  }

  store_wifi_credentials("WiFi-2.4",
                         "180898Delia!"); // will be replaced by SoftAP method
  start_wifi(true); */

  WiFi.begin("WiFi-2.4", "180898Delia!");
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    logger.info("WiFi connection established!");

    return;
  }
}

static void createWebServer()
{
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              request->send(SPIFFS, "/index.html", String(), false,
                            [](const String &var)
                            {
                              if (var == "NETWORKS")
                              {
                                int    no_networks = WiFi.scanNetworks();
                                String result      = "";

                                for (int i = 0; i < no_networks; ++i)
                                {
                                  result +=
                                      "<option class=\"list-group-item\">";
                                  result += WiFi.SSID(i);
                                  result += "</option>";
                                }

                                return result;
                              }

                              return String();
                            });
            });

  server.begin();

  logger.info("Web Server is up!");
}

void setup()
{
  Serial.begin(115200);
  if (!SPIFFS.begin())
  {
    logger.error("Couldn't mount filesystem");
  }

  startWifi();
  createWebServer();
}

void loop()
{
  delay(30 * 1000);
}