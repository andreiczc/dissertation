#include <Arduino.h>
#include <WiFi.h>

#include "crypto_utils.h"
#include "esp_log.h"
#include "spiffs_utils.h"
#include "web3_client.h"

#define ENV_SSID     "WiFi-2.4"
#define ENV_WIFI_KEY "180898Delia!"

// CONSTANTS
static constexpr auto *TAG         = "MAIN";
static constexpr auto *INFURA_HOST = "ropsten.infura.io";
static constexpr auto *INFURA_PATH = "/v3/8297ac153d3948b78c03d2aff759abef";
static constexpr auto *CONTRACT_ADDRESS =
    "0x14cAfcCc3c0857A9281A22E4b74E770A99c29cB1";
static std::string dataString =
    "0x4b2c190d00000000000000000000000000000000000000000000000000000000000000"
    "400000000000000000000000000000000000000000000000000000000000000039000000"
    "000000000000000000000000000000000000000000000000000000000868756d69646974"
    "79000000000000000000000000000000000000000000000000";

// MEMBERS
static Web3Client client(INFURA_HOST, INFURA_PATH);

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting setup");

  WiFi.begin(ENV_SSID, ENV_WIFI_KEY);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
  }

  ESP_LOGI(TAG, "WiFi connection successfull");
  client.callContract(CONTRACT_ADDRESS, dataString);
}

void loop()
{
  // put your main code here, to run repeatedly:
}