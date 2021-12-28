#include "tinycbor.h"

#define byte uint8_t

static const int OBJECT_ID   = 3301;
static const int RESOURCE_ID = 5100;
static int       INSTANCE_ID = 0;
static int       BUFFER_SIZE = 128;
static uint8_t   buffer[128];

static int register_to_network()
{
  // broadcast to mqtt
  // orchestrate using admin console
  // MOCK

  return 0;
}

static int get_time()
{
  // get time request
  // MOCK
  static int currTime = 0;

  return currTime++;
}

static void cbor_encode(uint8_t *payload, int payload_size)
{
  CborEncoder encoder;
  CborEncoder map_encoder;
  CborEncoder array_encoder;

  memset(payload, 0, payload_size); // empty the buffer

  cbor_encoder_init(&encoder, payload, payload_size, 0);
  cbor_encoder_create_map(&encoder, &map_encoder, 2); // timestamp + values

  cbor_encode_text_stringz(&map_encoder, "timestamp");
  cbor_encode_int(&map_encoder, get_time());

  cbor_encode_text_stringz(&map_encoder, "values");
  cbor_encoder_create_array(&map_encoder, &array_encoder, 1);

  add_value(&array_encoder, RESOURCE_ID, get_mock_value());

  cbor_encoder_close_container(&map_encoder, &array_encoder);
  cbor_encoder_close_container(&encoder, &map_encoder);
}

static void add_value(CborEncoder *parent_encoder, int resource_id, float value)
{
  CborEncoder innerEncoder;

  cbor_encoder_create_map(parent_encoder, &innerEncoder, 5);

  cbor_encode_text_stringz(&innerEncoder, "objectId");
  cbor_encode_float(&innerEncoder, 1);

  cbor_encode_text_stringz(&innerEncoder, "instanceId");
  cbor_encode_float(&innerEncoder, 2);

  cbor_encode_text_stringz(&innerEncoder, "resourceId");
  cbor_encode_float(&innerEncoder, 3);

  cbor_encode_text_stringz(&innerEncoder, "datatype");
  cbor_encode_text_stringz(&innerEncoder, "Float");

  cbor_encode_text_stringz(&innerEncoder, "value");
  cbor_encode_float(&innerEncoder, value);

  cbor_encoder_close_container(parent_encoder, &innerEncoder);
}

static float get_mock_value()
{
  return 2.0f;
}

void setup()
{
  INSTANCE_ID = register_to_network();

  Serial.begin(9600);
}

void loop()
{
  cbor_encode(buffer, BUFFER_SIZE);
  for (byte i = 0; i < BUFFER_SIZE; ++i)
  {
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  delay(5000);
}
