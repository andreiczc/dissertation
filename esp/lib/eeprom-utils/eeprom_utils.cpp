#include "eeprom_utils.h"

#include <EEPROM.h>

using namespace eeprom;

Utils::Utils(int size) noexcept
{
  if (size < 64)
  {
    size = 64;
  }

  this->eepromSize = size;
  EEPROM.begin(size);
}

std::pair<String, String> Utils::readWifiCredentials() const
{
  String ssid;
  String pass;
  auto   idx       = 0;
  char   currChar  = 0;
  auto   sizeLimit = 16;

  while ((currChar = EEPROM.read(idx++)) != ' ' && sizeLimit--)
  {
    ssid += currChar;
  }

  sizeLimit = 16;
  while ((currChar = EEPROM.read(idx++)) != ' ' && sizeLimit--)
  {
    pass += currChar;
  }

  return std::make_pair(ssid, pass);
}

void Utils::storeWifiCredentials(const String &ssid, const String &pass)
{
  for (auto i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), ' ');

  for (auto i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(ssid.length() + i + 1, pass[i]);
  }
  EEPROM.write(ssid.length() + pass.length() + 1, ' ');

  EEPROM.commit();
}

void Utils::emptyEeprom(bool commit) const
{
  for (auto i = 0; i < this->eepromSize; ++i)
  {
    EEPROM.write(i, 0);
  }

  if (commit)
  {
    EEPROM.commit();
  }
}