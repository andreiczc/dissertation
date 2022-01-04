#include "logger.h"
#include "net_utils.h"

void setup()
{
  Serial.begin(115200);

  NetUtils::startWifi();
}

void loop()
{
  delay(30 * 1000);
}