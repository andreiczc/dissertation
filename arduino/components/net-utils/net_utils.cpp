#include "net_utils.h"
#include "eeprom_utils.h"
#include "logger.h"

#include <FS.h>

static String createNetworkList()
{
  auto   no_networks = WiFi.scanNetworks();
  String result      = "";

  for (auto i = 0; i < no_networks; ++i)
  {
    result += "<option class=\"list-group-item\">";
    result += WiFi.SSID(i);
    result += "</option>";
  }

  return result;
}

static String processor(const String &var)
{
  if (var == "NETWORKS")
  {
    return createNetworkList();
  }

  return String();
}

static AsyncWebServer createWebServer()
{
  const auto *logger = Logger::getInstance();

  if (!SPIFFS.begin())
  {
    logger->error("Couldn't mount filesystem");
  }

  auto server = AsyncWebServer(80);
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              request->send(SPIFFS, "/index.html", String(), false, processor);
            });

  server.begin();

  logger->info("Web Server is up!");

  return server;
}

static int8_t startWifiStoredCredentials()
{
  const auto eepromUtils = eeprom::Utils(128);
  const auto credentials = eepromUtils.readWifiCredentials();

  WiFi.mode(WIFI_STA);
  WiFi.begin(credentials.first, credentials.second);

  return WiFi.waitForConnectResult();
}

static void startWifiAp()
{
  const auto *logger = Logger::getInstance();

  WiFi.softAP("sensor-1f2a46kg", "1f2a46kg");
  logger->info("Soft AP is up!");

  const auto webServer = createWebServer();

  while (true)
  {
    logger->info("Waiting for credentials");
    delay(60 * 1000);
  }
}

void NetUtils::startWifi()
{
  /* if (startWifiStoredCredentials() == WL_CONNECTED)
  {
    Logger::getInstance()->info("WiFi connection has been established!");
    return;
  } */

  startWifiAp();
}