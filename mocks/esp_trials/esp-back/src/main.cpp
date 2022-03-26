#include <Arduino.h>
#include <PubSubClient.h>

#include "net_utils.h"

PubSubClient client;

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