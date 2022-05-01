#include "smart_obj.h"

#include <cbor.h>

using namespace ipso;

static void encodeValue(CborEncoder *const      parentEncoder,
                        const SmartObjectValue &value)
{
  CborEncoder innerEncoder;

  cbor_encoder_create_map(parentEncoder, &innerEncoder, 5);

  cbor_encode_text_stringz(&innerEncoder, "objectId");
  cbor_encode_int(&innerEncoder, value.getObjectId());

  cbor_encode_text_stringz(&innerEncoder, "instanceId");
  cbor_encode_int(&innerEncoder, value.getInstanceId());

  cbor_encode_text_stringz(&innerEncoder, "resourceId");
  cbor_encode_int(&innerEncoder, value.getResourceId());

  const auto valuePair = value.getValue();
  cbor_encode_text_stringz(&innerEncoder, "datatype");
  cbor_encode_text_stringz(&innerEncoder, valuePair.first.c_str());

  cbor_encode_text_stringz(&innerEncoder, "value");
  cbor_encode_text_stringz(&innerEncoder, valuePair.second.c_str());

  cbor_encoder_close_container(parentEncoder, &innerEncoder);
}

String SmartObject::cbor()
{
  CborEncoder parentEncoder;
  CborEncoder mapEncoder;
  CborEncoder arrayEncoder;
  uint8_t     buffer[128];

  cbor_encoder_init(&parentEncoder, buffer, 128, 0);
  cbor_encoder_create_map(&parentEncoder, &mapEncoder, 2); // timestamp + values

  cbor_encode_text_stringz(&mapEncoder, "timestamp");
  cbor_encode_int(&mapEncoder, 1); // TODO add time lib

  cbor_encode_text_stringz(&mapEncoder, "values");
  cbor_encoder_create_array(&mapEncoder, &arrayEncoder, this->values.size());

  for (const auto &value : values)
  {
    encodeValue(&arrayEncoder, value);
  }

  cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
  cbor_encoder_close_container(&parentEncoder, &mapEncoder);

  return String((char *)buffer);
}
