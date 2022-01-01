#include "fault_handler.h"

#include "Arduino.h"

void signal_fault(const char *message)
{
  Serial.println(message);
  delay(60 * 1000);
}