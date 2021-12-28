#include "tinycbor.h"

static const int OBJECT_ID   = 3301;
static int       INSTANCE_ID = 0;
static int       RESOURCE_ID = 5100;

static int register_to_network()
{
  // broadcast to mqtt
  // orchestrate using admin console

  return 0;
}

static void add_resource(CborEncoder *parent_encoder, int resource_id,
                         void (*rsc_function)(CborEncoder *))
{
  CborEncoder innerEncoder;

  cbor_encoder_create_map(parent_encoder, &innerEncoder, 5);

  cbor_encode_text_stringz(&innerEncoder, "objectId");
  cbor_encode_int(&innerEncoder, OBJECT_ID);

  cbor_encode_text_stringz(&innerEncoder, "instanceId");
  cbor_encode_int(&innerEncoder, INSTANCE_ID);

  cbor_encode_text_stringz(&innerEncoder, "resourceId");
  cbor_encode_int(&innerEncoder, RESOURCE_ID);

  rsc_function(&innerEncoder);

  cbor_encoder_close_container(parent_encoder, &innerEncoder);
}

static void retrieveSensorValue(CborEncoder *encoder)
{
  cbor_encode_text_stringz(encoder, "datatype");
  cbor_encode_text_stringz(encoder, "Float");

  cbor_encode_text_stringz(encoder, "value");
  cbor_encode_float(encoder, 12.2f);
}

void setup()
{
  INSTANCE_ID = register_to_network();
}

void loop() {}