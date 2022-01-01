#include <ESP8266WiFi.h>

#include "logger.h"
#include "wifi_wrapper.h"

enum ErrorCodes
{
  WIFI_ERR = 1
};

WifiWrapper::WifiWrapper(const char *ssid, const char *pass,
                         const char *broker_url)
    : logger(Serial)
{
  if (connect_to_wifi(ssid, pass))
  {
    logger.error("WiFi connection couldn't be established");
  }

  logger.info("WiFi connection successful");
}

int WifiWrapper::connect_to_wifi(const char *ssid, const char *pass) const
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
  }

  WiFi.begin(ssid, pass);
  int ctr = 5;
  while (WiFi.status() != WL_CONNECTED && ctr--)
  {
    delay(5000);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    return ErrorCodes::WIFI_ERR;
  }

  return 0;
}

int WifiWrapper::register_to_network(const char *broker_url)
{
  this->instance_number = 0;
}

int WifiWrapper::get_instance_number() const
{
  return this->instance_number;
}