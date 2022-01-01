#ifndef _LOGGER_H
#define _LOGGER_H

#include "Stream.h"

class Logger
{
public:
  Logger(Stream &stream);

  void info(const char *message) const;

  void error(const char *message) const;

private:
  Stream &impl;
};

#endif // _LOGGER_H