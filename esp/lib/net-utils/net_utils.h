#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"

#include <memory>

class NetUtils
{
public:
  static std::shared_ptr<NetUtils> getInstance();

  void startWifi();

  esp_mqtt_client_handle_t initMqttConnection();

  EventGroupHandle_t getWifiEventGroup() const;

private:
  explicit NetUtils();

  EventGroupHandle_t wifiEventGroup;

  static const char               *TAG;
  static std::shared_ptr<NetUtils> instance;

  void startWiFiSta();
};

#endif // _NET_UTILS_H