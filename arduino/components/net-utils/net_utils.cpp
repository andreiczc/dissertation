#include "net_utils.h"
#include "eeprom_utils.h"
#include "logger.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPSServer.hpp>
#include <SPIFFS.h>
#include <SSLCert.hpp>
#include <WiFi.h>

static String createNetworkList()
{
  const auto *logger = Logger::getInstance();

  logger->info("Will now turn on STATION mode and commence network scan!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(3000);

  const auto noNetworks = WiFi.scanNetworks();
  logger->info("Found " + String(noNetworks) + " networks");

  String result = "";

  for (auto i = 0; i < noNetworks; ++i)
  {
    result += "<option class=\"list-group-item\">";
    result += WiFi.SSID(i);
    result += "</option>";
  }

  return result;
}

static AsyncWebServer createWebServer(const String &networkList)
{
  const auto *logger = Logger::getInstance();

  if (!SPIFFS.begin())
  {
    logger->error("Couldn't mount filesystem");
  }

  auto server = AsyncWebServer(80);
  server.on("/", HTTP_GET,
            [&logger, &networkList](AsyncWebServerRequest *request)
            {
              logger->info("Received GET request on /");
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
            [&logger](AsyncWebServerRequest *request)
            {
              logger->info("Received GET request on /bootstrap.css");
              request->send(SPIFFS, "/bootstrap.css", "text/css");
            });
  server.on("/jquery.js", HTTP_GET,
            [&logger](AsyncWebServerRequest *request)

            {
              logger->info("Received GET request on /jquery.js");
              request->send(SPIFFS, "/jquery.js", "text/javascript");
            });
  server.on("/popper.js", HTTP_GET,
            [&logger](AsyncWebServerRequest *request)
            {
              logger->info("Received GET request on /popper.js");
              request->send(SPIFFS, "/popper.js", "text/javascript");
            });
  server.on("/bootstrap.js", HTTP_GET,
            [&logger](AsyncWebServerRequest *request)
            {
              logger->info("Received GET request on /bootstrap.js");
              request->send(SPIFFS, "/bootstrap.js", "text/javascript");
            });

  server.begin();

  logger->info("Web Server is up!");

  return server;
}

static httpsserver::HTTPSServer createWebServerSecure()
{
  using namespace httpsserver;

  const auto *logger = Logger::getInstance();

  auto       sslCert = SSLCert();
  const auto returnCode =
      createSelfSignedCert(sslCert, KEYSIZE_1024, "CN=ESP32,O=ase,C=RO");

  if (returnCode)
  {
    logger->error("Couldn't create https server");
  }

  auto serverRunning = true;

  auto server = HTTPSServer(&sslCert);
  auto node =
      ResourceNode("/", "GET", [](HTTPRequest *req, HTTPResponse *res) {});

  server.registerNode(&node);

  return server;
}

static void startWifiAp()
{
  const auto *logger = Logger::getInstance();

  const auto networkList = createNetworkList();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("sensor-1f2a46kg", "1f2a46kg");
  logger->info("Soft AP is up!");

  const auto webServer = createWebServer(networkList);

  while (true)
  {
    logger->info("Waiting for credentials");
    delay(60 * 1000);
  }
}

static int8_t startWifiStoredCredentials()
{
  eeprom::Utils eepromUtils(128);
  const auto    credentials = eepromUtils.readWifiCredentials();

  const auto *instance = Logger::getInstance();
  instance->info("SSID: " + credentials.first);
  instance->info("Pass: " + credentials.second);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(credentials.first.c_str(), credentials.second.c_str());

  return WiFi.waitForConnectResult();
}

void NetUtils::startWifi()
{
  const auto networkList = createNetworkList();

  if (startWifiStoredCredentials() == WL_CONNECTED)
  {
    Logger::getInstance()->info("WiFi connection has been established!");

    const auto httpWebServer = createWebServer(networkList);

    return;
  }

  // startWifiAp();
}