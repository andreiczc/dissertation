#include <Arduino.h>
#include <WiFi.h>

#include "crypto_utils.h"
#include "esp_log.h"
#include "net_utils.h"
#include "web3_client.h"

// CONSTANTS
static constexpr auto *TAG = "MAIN";
static constexpr auto *CONTRACT_ADDRESS =
    "0x14cAfcCc3c0857A9281A22E4b74E770A99c29cB1";
static std::string dataString =
    "0x4b2c190d00000000000000000000000000000000000000000000000000000000000000"
    "400000000000000000000000000000000000000000000000000000000000000039000000"
    "000000000000000000000000000000000000000000000000000000000868756d69646974"
    "79000000000000000000000000000000000000000000000000";

// MEMBERS
static esp_mqtt_client_handle_t mqttClient;

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting setup");

  NetUtils::startWifi();
  // NetUtils::attestDevice();
  mqttClient = NetUtils::initMqttConnection();
}

void loop()
{
  // put your main code here, to run repeatedly:
}