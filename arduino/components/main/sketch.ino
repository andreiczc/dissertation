#include "logger.h"

void setup()
{
  Serial.begin(115200);

  Logger::getInstance()->info("Program has started!");
}

void loop()
{
  delay(30 * 1000);
}