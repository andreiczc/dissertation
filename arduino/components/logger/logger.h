#ifndef _LOGGER_H
#define _LOGGER_H

#include "Stream.h"

class Logger
{
public:
  explicit Logger(Stream &stream) noexcept : impl(stream) {}

  void info(const String &message) const;
  void error(const String &message) const;

private:
  Stream &impl;
};

#endif // _LOGGER_H