#ifndef _SENSOR_UTILS_H
#define _SENSOR_UTILS_H

#include "Arduino.h"
#include "gas_sensor.h"
#include "sensor_types.h"
#include "vibration_sensor.h"
#include <memory>

class SensorUtils
{
public:
  static std::shared_ptr<SensorUtils> getInstance();

  float querySensor(SensorType type);

private:
  VibrationSensor vibrationSensor;
  GasSensor       gasSensor;

  float oldTemperature = 0;
  float oldHumidity    = 0;

  explicit SensorUtils();

  float queryDht(SensorType type);

  static std::shared_ptr<SensorUtils> instance;
};

#endif // _SENSOR_UTILS_H