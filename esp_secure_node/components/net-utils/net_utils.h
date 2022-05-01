#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include "mqtt_client.h"
#include <ESPAsyncWebServer.h>

class NetUtils
{
public:
  static void startWifi();

  static void attestDevice();

  static esp_mqtt_client_handle_t initMqttConnection();

  static void publishAll(esp_mqtt_client_handle_t &client);

  static std::unique_ptr<AsyncWebServer> startManagementServer();
};

#endif // _NET_UTILS_H