#include "net_utils.h"
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

void NetUtils::createWebServer(AsyncWebServer *server)
{
  Logger *logger = Logger::getInstance();

  if (!SPIFFS.begin())
  {
    logger->error("Couldn't mount filesystem");
  }

  server->on("/", HTTP_GET,
             [](AsyncWebServerRequest *request) {
               request->send(SPIFFS, "/index.html", String(), false, processor);
             });

  server->begin();

  logger->info("Web Server is up!");
}

void NetUtils::startWifi()
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
    Logger::getInstance()->info("WiFi connection has been established!");
    return;
  }
}