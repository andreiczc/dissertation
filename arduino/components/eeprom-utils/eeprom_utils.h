#ifndef _EEPROM_UTILS_H
#define _EEPROM_UTILS_H

namespace eeprom
{
class Utils
{
public:
  explicit Utils(int size) noexcept;
  void storeWifiCredentials(const String &ssid, const String &pass);
  std::pair<String, String> readWifiCredentials() const;
  void                      emptyEeprom(bool commit = false) const;

private:
  int eepromSize;
};
} // namespace eeprom

#endif // _EEPROM_UTILS_H