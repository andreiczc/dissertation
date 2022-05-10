#ifndef _SENSOR_UTILS_H
#define _SENSOR_UTILS_H

#include "sensor_types.h"
#include <Arduino.h>

class SensorUtils
{
public:
  static float querySensor(SensorType type);
};

#endif // _SENSOR_UTILS_H