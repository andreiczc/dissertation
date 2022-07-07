#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include "mqtt_client.h"

class NetUtils
{
public:
  static void startWifi();

  static void attestDevice();

  static esp_mqtt_client_handle_t initMqttConnection();
};

#endif // _NET_UTILS_H