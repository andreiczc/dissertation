#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <tinycbor.h>

#include "logger.h"

#define byte uint8_t

static const int      OBJECT_ID   = 3301;
static const int      RESOURCE_ID = 5100;
static int            INSTANCE_ID = 0;
static int            BUFFER_SIZE = 128;
static const int      EEPROM_SIZE = 128;
static uint8_t        buffer[128];
static Logger         logger(Serial);
static String         ssid    = "";
static String         pass    = "";
static String         content = "";
static int            status_code;
static AsyncWebServer server(80);

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

static void read_wifi_credentials()
{
  int  idx        = 0;
  char curr_char  = 0;
  int  size_limit = 16;

  while ((curr_char = EEPROM.read(idx++)) != ' ' && size_limit--)
  {
    ssid += curr_char;
  }

  size_limit = 16;
  while ((curr_char = EEPROM.read(idx++)) != ' ' && size_limit--)
  {
    pass += curr_char;
  }
}

static void store_wifi_credentials(const String &ssid, const String &pass)
{
  for (size_t i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), ' ');

  for (size_t i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(ssid.length() + i + 1, pass[i]);
  }
  EEPROM.write(ssid.length() + pass.length() + 1, ' ');

  EEPROM.commit();
}

// this function takes quite a while to execute
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

static void clear_eeprom(bool commit = false)
{
  for (int i = 0; i < EEPROM_SIZE; ++i)
  {
    EEPROM.write(i, 0);
  }

  if (commit)
  {
    EEPROM.commit();
  }
}

static void start_wifi(bool in_memory = false)
{
  /* if (!in_memory)
  {
    read_wifi_credentials();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    logger.info("WiFi connection established!");
    empty_wifi_credentials();
    return;
  }

  store_wifi_credentials("WiFi-2.4",
                         "180898Delia!"); // will be replaced by SoftAP method
  start_wifi(true); */

  WiFi.begin("WiFi-2.4", "180898Delia!");
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    logger.info("WiFi connection established!");

    return;
  }
}

static String create_network_list()
{
  int    no_networks = WiFi.scanNetworks();
  String result      = "";

  for (int i = 0; i < no_networks; ++i)
  {
    result += "<option class=\"list-group-item\">";
    result += WiFi.SSID(i);
    result += "</option>";
  }

  return result;
}

static String processor(const String &var)
{
  if (var == "NETWORKS")
  {
    return create_network_list();
  }

  return String();
}

static void create_web_server()
{
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              request->send(SPIFFS, "/index.html", String(), false, processor);
            });

  server.on(
      "/setCredentials", HTTP_POST,
      [](AsyncWebServerRequest *request)
      {
        if (!request->hasArg("plain"))
        {
          request->send(400, "text/plain", "Bad request");

          return;
        }

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, request->arg("plain"));

        logger.info(doc["ssid"]);
        logger.info(doc["pass"]);

        request->send(
            200, "text/plain",
            "WiFi settings have been saved. The sensor will now restart!");
      });

  server.begin();

  logger.info("Web Server is up!");
}

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    logger.error("Couldn't mount filesystem");
  }
  // EEPROM.begin(EEPROM_SIZE);

  start_wifi();
  create_web_server();
}

void loop()
{
  delay(30 * 1000);
}