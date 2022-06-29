#include <Arduino.h>
#include <WiFi.h>

#include "esp_log.h"
#include "net_utils.h"

// CONSTANTS
static constexpr auto *TAG = "MAIN";

// MEMBERS
static esp_mqtt_client_handle_t        mqttClient;
static std::unique_ptr<AsyncWebServer> managementServer(nullptr);

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting setup");

  NetUtils::startWifi();
  NetUtils::attestDevice();
  managementServer = NetUtils::startManagementServer();
  mqttClient       = NetUtils::initMqttConnection();
}

void loop()
{
  ESP_LOGI(TAG, "Looping");

  NetUtils::publishAll(mqttClient);

  delay(60000);
}