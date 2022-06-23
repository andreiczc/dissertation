#ifndef _SENSOR_UTILS_H
#define _SENSOR_UTILS_H

#include "Arduino.h"
#include "dht.h"
#include "gas_sensor.h"
#include "sensor_types.h"
#include "vibration_sensor.h"

class SensorUtils
{
public:
  explicit SensorUtils();

  float querySensor(SensorType type);

private:
  DHT             dht;
  VibrationSensor vibrationSensor;
  GasSensor       gasSensor;
};

#endif // _SENSOR_UTILS_H