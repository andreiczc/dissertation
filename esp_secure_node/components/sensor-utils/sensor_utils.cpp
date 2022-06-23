#include "sensor_utils.h"

static constexpr auto VIBRATION_PIN = 32;
static constexpr auto DHT_PIN       = 33;
static constexpr auto DHT_TYPE      = DHT11;
static constexpr auto GAS_PIN       = 35;

SensorUtils::SensorUtils()
    : dht(DHT_PIN, DHT_TYPE), vibrationSensor(VIBRATION_PIN), gasSensor(GAS_PIN)
{
  dht.begin();
}

float SensorUtils::querySensor(SensorType type)
{
  switch (type)
  {
  case SensorType::TEMPERATURE:
    if (!dht.read())
    {
      return 0;
    }

    return dht.readTemperature();
  case SensorType::HUMIDITY:
    if (!dht.read())
    {
      return 0;
    }

    return dht.readHumidity();
  case SensorType::GAS:
    return gasSensor.readValue();
  case SensorType::VIBRATION:
    return vibrationSensor.vibrationPresent();
  default:
    return 0;
  }
}