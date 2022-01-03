#ifndef _EEPROM_UTILS_H
#define _EEPROM_UTILS_H

#include <Arduino.h>

namespace eeprom
{
class Utils
{
public:
  explicit Utils(int size) noexcept;
  void storeWifiCredentials(const String &ssid, const String &pass);
  std::pair<String, String> readWifiCredentials() const;
  void                      emptyEeprom(bool commit = false) const;

  virtual ~Utils() noexcept = default;
  Utils(const Utils &utils) = delete;
  Utils &operator=(const Utils &utils) = delete;

private:
  int eepromSize;
};
} // namespace eeprom

#endif // _EEPROM_UTILS_H