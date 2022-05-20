#ifndef _SMART_OBJ_H
#define _SMART_OBJ_H

#include <Arduino.h>
#include <memory>
#include <vector>

#include "smart_obj_value.h"

namespace ipso
{
class SmartObject
{
public:
  explicit SmartObject() noexcept = default;

  std::unique_ptr<uint8_t[]> cbor(size_t &size) noexcept;

  void addValue(SmartObjectValue value) { this->values.push_back(value); }

private:
  std::vector<SmartObjectValue> values;
};
} // namespace ipso

#endif // _SMART_OBJ_H