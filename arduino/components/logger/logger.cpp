#include "logger.h"

Logger::Logger(Stream &stream) : impl(stream) {}

void Logger::error(const char *message) const
{
  while (true)
  {
    impl.print("[ERROR] ");
    impl.print(message);
    impl.println();

    delay(30 * 1000);
  }
}

void Logger::info(const char *message) const
{
  impl.print("[INFO] ");
  impl.print(message);
  impl.println();
}