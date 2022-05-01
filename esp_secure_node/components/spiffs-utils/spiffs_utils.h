#ifndef _SPIFFS_UTILS_H
#define _SPIFFS_UTILS_H

#include "Arduino.h"
#include <memory>

class SpiffsUtils
{
public:
  static std::shared_ptr<SpiffsUtils> getInstance();

  String readText(const String &path);

  void writeText(const String &path, const String &content);

private:
  explicit SpiffsUtils();

  static const char                  *TAG;
  static std::shared_ptr<SpiffsUtils> instance;
};

#endif // _SPIFFS_UTILS_H