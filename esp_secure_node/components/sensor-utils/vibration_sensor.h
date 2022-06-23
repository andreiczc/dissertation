#ifndef _VIBRATION_SENSOR_H
#define _VIBRATION_SENSOR_H

#include <Arduino.h>

class VibrationSensor
{
public:
  explicit VibrationSensor(short pinNumber) : pinNumber(pinNumber)
  {
    pinMode(this->pinNumber, INPUT);
  }

  boolean vibrationPresent() { return digitalRead(this->pinNumber) >= 1; }

private:
  short pinNumber;
};

#endif // _VIBRATION_SENSOR_H