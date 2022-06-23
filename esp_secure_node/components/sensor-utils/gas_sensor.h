#ifndef _GAS_SENSOR_H
#define _GAS_SENSOR_H

class GasSensor
{
public:
  explicit GasSensor(short pinNumber) : pinNumber(pinNumber) {}

  int readValue() { return analogRead(this->pinNumber); }

private:
  short pinNumber;
};

#endif // _GAS_SENSOR_H