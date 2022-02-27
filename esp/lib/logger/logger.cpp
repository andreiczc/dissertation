#include "logger.h"

#include <Arduino.h>

Logger *Logger::instance = nullptr;

Logger *Logger::getInstance()
{
  if (instance == nullptr)
  {
    instance = new Logger(Serial);
  }

  return instance;
}

void Logger::error(const String &message) const
{
  while (true)
  {
    impl.print("[ERROR] ");
    impl.print(message);
    impl.println();

    delay(30 * 1000);
  }
}

void Logger::info(const String &message) const
{
  impl.print("[INFO] ");
  impl.print(message);
  impl.println();
}