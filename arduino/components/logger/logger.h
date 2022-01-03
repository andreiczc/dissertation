#ifndef _LOGGER_H
#define _LOGGER_H

#include "Stream.h"

class Logger
{
public:
  static Logger *getInstance();
  void           info(const String &message) const;
  void           error(const String &message) const;

private:
  explicit Logger(Stream &stream) noexcept : impl(stream) {}

  Stream        &impl;
  static Logger *instance;
};

#endif // _LOGGER_H