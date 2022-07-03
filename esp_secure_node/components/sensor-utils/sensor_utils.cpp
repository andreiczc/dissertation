#include "sensor_utils.h"
#include "dht.h"

static constexpr auto *TAG           = "SENSOR_UTILS";
static constexpr auto  VIBRATION_PIN = 32;
static constexpr auto  DHT_PIN       = 33;
static constexpr auto  GAS_PIN       = 35;

std::shared_ptr<SensorUtils> SensorUtils::instance =
    std::shared_ptr<SensorUtils>(nullptr);

std::shared_ptr<SensorUtils> SensorUtils::getInstance()
{
  if (!instance)
  {
    instance.reset(new SensorUtils());
  }

  return instance;
}

static bool inBounds(float value, SensorType type)
{
  switch (type)
  {
  case SensorType::TEMPERATURE:
    return value >= 15 && value <= 40;
  case SensorType::HUMIDITY:
    return value >= 10 && value <= 90;
  default:
    return true;
  }
}

SensorUtils::SensorUtils() : vibrationSensor(VIBRATION_PIN), gasSensor(GAS_PIN)
{
  float temperature = 0;
  float humidity    = 0;

  while (!inBounds(temperature, SensorType::TEMPERATURE) ||
         !inBounds(humidity, SensorType::HUMIDITY))
  {
    dht_read_float_data(DHT_TYPE_DHT11, GPIO_NUM_33, &humidity, &temperature);
  }

  this->oldTemperature = temperature;
  this->oldHumidity    = oldHumidity;
}

float SensorUtils::queryDht(SensorType type)
{
  float temperature;
  float humidity;

  dht_read_float_data(DHT_TYPE_DHT11, GPIO_NUM_33, &humidity, &temperature);

  if (inBounds(temperature, SensorType::TEMPERATURE) &&
      inBounds(humidity, SensorType::HUMIDITY))
  {
    this->oldTemperature = temperature;
    this->oldHumidity    = humidity;
    ESP_LOGI(TAG, "Using new DHT readings");
  }
  else
  {
    temperature = this->oldTemperature;
    humidity    = this->oldHumidity;
    ESP_LOGI(TAG, "Using old DHT readings");
  }

  ESP_LOGI(TAG, "DHT: temperature: %f; humidity: %f", temperature, humidity);

  return type == SensorType::TEMPERATURE ? temperature : humidity;
}

float SensorUtils::querySensor(SensorType type)
{
  switch (type)
  {
  case SensorType::TEMPERATURE:
  case SensorType::HUMIDITY:
    return queryDht(type);
  case SensorType::GAS:
    return gasSensor.readValue();
  case SensorType::VIBRATION:
    return vibrationSensor.vibrationPresent();
  default:
    return 0;
  }
}