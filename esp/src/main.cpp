#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#include "net_utils.h"

WiFiClientSecure _;
PubSubClient     client(_);

void setup()
{
  Serial.begin(115200);
  log_i("Program has started!");

  NetUtils::startWifi();
}

void loop()
{
  log_i("Looping");
  delay(30 * 1000);
}