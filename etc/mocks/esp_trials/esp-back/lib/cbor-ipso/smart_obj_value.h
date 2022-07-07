#ifndef _SMART_OBJ_VALUE_H
#define _SMART_OBJ_VALUE_H

namespace ipso
{
class SmartObjectValue
{
public:
  explicit SmartObjectValue(int objectId, int instanceId, int resourceId,
                            const String &value) noexcept
      : objectId(objectId), instanceId(instanceId), resourceId(resourceId),
        dataType("String"), value(value)
  {
  }

  explicit SmartObjectValue(int objectId, int instanceId, int resourceId,
                            float value) noexcept
      : objectId(objectId), instanceId(instanceId), resourceId(resourceId),
        dataType("String"), value(String(value))
  {
  }

  explicit SmartObjectValue(int objectId, int instanceId, int resourceId,
                            float value) noexcept
      : objectId(objectId), instanceId(instanceId), resourceId(resourceId),
        dataType("String"), value(String(value))
  {
  }

  int                       getObjectId() const { return this->objectId; }
  int                       getInstanceId() const { return this->instanceId; }
  int                       getResourceId() const { return this->resourceId; }
  std::pair<String, String> getValue() const
  {
    return std::make_pair(dataType, value);
  }

private:
  int    objectId;
  int    instanceId;
  int    resourceId;
  String dataType;
  String value;
};
} // namespace ipso

#endif // _SMART_OBJ_VALUE_H