#include <FreeRTOS.h>

#include "net_utils.h"

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
  log_i("Will now turn on STATION mode and commence network scan!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(3000);

  const auto noNetworks = WiFi.scanNetworks();
  log_i("Found %d networks", noNetworks);

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
  if (!SPIFFS.begin())
  {
    log_e("Couldn't mount filesystem");
  }

  auto server = AsyncWebServer(80);
  server.on("/", HTTP_GET,
            [&networkList](AsyncWebServerRequest *request)
            {
              log_i("Received GET request on /");
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
              log_i("Received GET request on /bootstrap.css");
              request->send(SPIFFS, "/bootstrap.css", "text/css");
            });
  server.on("/jquery.js", HTTP_GET,
            [](AsyncWebServerRequest *request)

            {
              log_i("Received GET request on /jquery.js");
              request->send(SPIFFS, "/jquery.js", "text/javascript");
            });
  server.on("/popper.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              log_i("Received GET request on /popper.js");
              request->send(SPIFFS, "/popper.js", "text/javascript");
            });
  server.on("/bootstrap.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              log_i("Received GET request on /bootstrap.js");
              request->send(SPIFFS, "/bootstrap.js", "text/javascript");
            });

  server.begin();

  log_i("Web Server is up!");

  return server;
}

static httpsserver::HTTPSServer createWebServerSecure()
{
  using namespace httpsserver;

  auto sslCert = SSLCert();
  if (createSelfSignedCert(sslCert, KEYSIZE_1024, "CN=ESP32,O=ase,C=RO"))
  {
    log_e("Couldn't create https server");
  }

  auto server = HTTPSServer(&sslCert, 8081);
  auto node   = ResourceNode("/credentials", "POST",
                             [](HTTPRequest *req, HTTPResponse *res)
                             {
                             char buffer[256];
                             req->readChars(buffer, 256);
                             auto ssid = strtok(buffer, " ");
                             auto pass = strtok(nullptr, " ");

                             WiFi.mode(WIFI_STA);
                             WiFi.begin(ssid, pass);
                             ESP.restart();
                             });

  server.registerNode(&node);

  return server;
}

static void startWifiAp()
{
  const auto networkList = createNetworkList();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("sensor-1f2a46kg", "1f2a46kg");
  log_i("Soft AP is up!");

  const auto webServer       = createWebServer(networkList);
  auto       webServerSecure = createWebServerSecure();

  webServerSecure.start();
  if (!webServerSecure.isRunning())
  {
    log_e("Https server is not up");
  }

  log_i("Web servers are running");

  while (true)
  {
    webServerSecure.loop();
    delay(1 * 1000);
  }
}

void NetUtils::startWifi()
{
  if (WiFi.begin() != WL_CONNECT_FAILED)
  {
    log_i("Connection successful");
    return;
  }

  startWifiAp();
}