#include "wifi_wrapper.h"
#include "logger.h"

WifiWrapper::WifiWrapper(const char *ssid, const char *pass,
                         const char *broker_url)
    : logger(Serial)
{
  connect_to_wifi(ssid, pass);
  Serial.print(WiFi.status());

  logger.info("WiFi connection successful");
}

wl_status_t WifiWrapper::connect_to_wifi(const char *ssid,
                                         const char *pass) const
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

  return WiFi.status();
}

int WifiWrapper::register_to_network(const char *broker_url)
{
  this->instance_number = 0;
}

int WifiWrapper::get_instance_number() const
{
  return this->instance_number;
}