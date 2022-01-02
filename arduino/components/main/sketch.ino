#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <GDBStub.h>

#include "tinycbor.h"

#include "logger.h"

#define byte uint8_t

static const int OBJECT_ID   = 3301;
static const int RESOURCE_ID = 5100;
static int       INSTANCE_ID = 0;
static int       BUFFER_SIZE = 128;
static uint8_t   buffer[128];
static Logger    logger(Serial);
static String    ssid = "";
static String    pass = "";

static size_t cbor_encode(uint8_t *const payload, int buffer_size)
{
  CborEncoder encoder;
  CborEncoder map_encoder;
  CborEncoder array_encoder;

  memset(payload, 0, buffer_size); // empty the buffer

  cbor_encoder_init(&encoder, payload, buffer_size, 0);
  cbor_encoder_create_map(&encoder, &map_encoder, 2); // timestamp + values

  cbor_encode_text_stringz(&map_encoder, "timestamp");
  cbor_encode_int(&map_encoder, 1);

  cbor_encode_text_stringz(&map_encoder, "values");
  cbor_encoder_create_array(&map_encoder, &array_encoder, 1);

  add_value(&array_encoder, RESOURCE_ID, 2.9f);

  cbor_encoder_close_container(&map_encoder, &array_encoder);
  cbor_encoder_close_container(&encoder, &map_encoder);

  return cbor_encoder_get_buffer_size(&encoder, payload);
}

static void add_value(CborEncoder *const parent_encoder, int resource_id,
                      float value)
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
  cbor_encode_double(&innerEncoder, 3.12);

  cbor_encoder_close_container(parent_encoder, &innerEncoder);
}

static void dump_hex(Stream &stream, const uint8_t *const buffer, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    stream.printf("%02x", buffer[i]);
  }
  stream.println();
}

static wl_status_t timed_wifi_status()
{
  short ctr = 30;
  while (WiFi.status() != WL_CONNECTED && ctr--)
  {
    delay(500);
  }

  return WiFi.status();
}

static void read_wifi_credentials()
{
  int  idx       = 0;
  char curr_char = 0;

  while ((curr_char = EEPROM.read(idx++)) != ' ' && curr_char != 0)
  {
    ssid += curr_char;
  }

  while ((curr_char = EEPROM.read(idx++)) != ' ' && curr_char != 0)
  {
    pass += curr_char;
  }
}

static void store_wifi_credentials(String ssid, String pass)
{
  for (size_t i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
  }

  EEPROM.write(ssid.length(), ' ');

  for (size_t i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(ssid.length() + i, pass[i]);
  }

  EEPROM.commit();
}

static void empty_wifi_credentials()
{
  for (int i = 0; i < ssid.length(); ++i)
  {
    ssid[i] = ' ';
  }

  for (int i = 0; i < pass.length(); ++i)
  {
    pass[i] = ' ';
  }
}

static void start_wifi()
{
  read_wifi_credentials();
  WiFi.begin(ssid, pass);
  if (timed_wifi_status() == WL_CONNECTED)
  {
    logger.info("WiFi connection established!");
    empty_wifi_credentials();
    return;
  }

  store_wifi_credentials("WiFi-2.4",
                         "180898Delia!"); // will be replaced by SoftAP method
  ESP.reset();
}

void setup()
{
  Serial.begin(115200);
  gdbstub_init();
  delay(30 * 1000);

  EEPROM.begin(128);

  start_wifi();
}

void loop()
{
  delay(30 * 1000);
}