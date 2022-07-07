#ifndef _SPIFFS_UTILS_H
#define _SPIFFS_UTILS_H

#include <memory>
#include <string>

class SpiffsUtils
{
public:
  static std::shared_ptr<SpiffsUtils> getInstance();

  std::string readText(const std::string &path);

private:
  explicit SpiffsUtils();

  static const char                  *TAG;
  static std::shared_ptr<SpiffsUtils> instance;
};

#endif // _SPIFFS_UTILS_H