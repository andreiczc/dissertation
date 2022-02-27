#include <Arduino.h>

#include "logger.h"
#include "net_utils.h"

static const auto *logger = Logger::getInstance();

void setup()
{
  Serial.begin(115200);
  logger->info("Program has started!");

  NetUtils::startWifi();
}

void loop()
{
  logger->info("Looping");
  delay(30 * 1000);
}