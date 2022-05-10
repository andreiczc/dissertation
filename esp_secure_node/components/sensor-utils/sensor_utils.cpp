#include "sensor_utils.h"

#include "esp_random.h"

float getTemperature()
{
  return 25.0 + (esp_random() % 120 * 0.01);
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

float SensorUtils::querySensor(SensorType type)
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