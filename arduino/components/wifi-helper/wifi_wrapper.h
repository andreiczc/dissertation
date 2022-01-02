#ifndef _WIFI_WRAPPER_H
#define _WIFI_WRAPPER_H

#include "logger.h"
#include <ESP8266WiFi.h>

class WifiWrapper
{
public:
  WifiWrapper(const char *ssid, const char *pass, const char *broker_url);
  int get_instance_number() const;

private:
  wl_status_t connect_to_wifi(const char *ssid, const char *pass) const;
  int         register_to_network(const char *broker_url);

  int instance_number;

  Logger logger;
};

#endif //  _WIFI_WRAPPER_H