#include "logger.h"
#include "net_utils.h"

AsyncWebServer *server = nullptr;

void setup()
{
  Serial.begin(115200);

  NetUtils::startWifi();

  server = new AsyncWebServer(80);
  NetUtils::createWebServer(server);
}

void loop()
{
  delay(30 * 1000);
}