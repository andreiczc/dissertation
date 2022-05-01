#include "sensor_utils.h"

#include "esp_random.h"

int getTemperature()
{
  return 25 + esp_random() % 25;
}

int getHumidity()
{
  return esp_random() % 99;
}

int getVibrations()
{
  return esp_random() % 1;
}

int getGases()
{
  return esp_random() % 1;
}

int SensorUtils::querySensor(SensorType type)
{
  switch (type)
  {
  case SensorType::TEMPERATURE:
    return getTemperature();
  case SensorType::HUMIDITY:
    return getHumidity();
  case SensorType::GAS:
    return getGases();
  case SensorType::VIBRATION:
    return getVibrations();
  default:
    return 0;
  }
}