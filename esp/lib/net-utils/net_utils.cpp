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
#include <WiFiClientSecure.h>

static constexpr char *CA_PATH = "/ca.crt";

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
  log_i("Attempting to start Soft AP");

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

static std::unique_ptr<char[]> read(const String &path)
{
  if (!SPIFFS.begin())
  {
    log_e("Couldn't mount filesystem");
  }

  auto       file   = SPIFFS.open(path);
  const auto size   = file.size();
  auto       buffer = std::unique_ptr<char[]>(new char[size + 1]);

  file.readBytes(buffer.get(), size);
  buffer[size] = 0;

  return buffer;
}

void NetUtils::startWifi()
{
  log_i("Attempting to connect to WiFi");

  if (WiFi.begin() != WL_CONNECT_FAILED)
  {
    log_i("Connection successful");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(5000);
    }

    return;
  }

  startWifiAp();
}

PubSubClient NetUtils::initClient()
{
  log_i("Attempting to create MQTT client");

  static WiFiClientSecure _;
  const auto              caCertContent = read(CA_PATH);
  _.setCACert(caCertContent.get());

  log_v("CA Cert is:\n %s", caCertContent.get());

  PubSubClient client(_);
  IPAddress    serverAddress((byte *)"192.168.26.39");
  client.setServer(serverAddress, 8883)
      .setCallback(
          [](const char *topic, byte *payload, unsigned int length) {

          })
      .connect("TestClient");

  if (!client.connected())
  {
    log_e("Error while connecting to broker.");
  }

  log_i("Connection to broker succeeded");

  return client;
}