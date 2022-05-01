#include "net_utils.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Arduino.h"
#include "WiFi.h"
#include "cJSON.h"
#include "crypto_utils.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "sensor_utils.h"
#include "spiffs_utils.h"
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <vector>

#define KEY_SIZE 32

// TODO add certificate checks in esp and server

// CONSTANTS
static constexpr auto *TAG         = "NET";
static constexpr auto *MQTT_SERVER = "mqtts://130.162.253.10:8883";
static const String    ATTESTATION_SERVER =
    "http://130.162.253.10:8080/attestation/";
static const String CLIENT_HELLO_ENDPOINT    = "clientHello";
static const String KEY_EXCHANGE_ENDPOINT    = "keyExchange";
static const String CLIENT_FINISHED_ENDPOINT = "clientFinished";

static const std::vector<SensorType> capabilities{
    SensorType::TEMPERATURE, SensorType::HUMIDITY, SensorType::GAS,
    SensorType::VIBRATION};

// MEMBERS
static uint8_t MQTT_PSK_KEY[KEY_SIZE] = "";

static String createNetworkList()
{
  ESP_LOGI(TAG, "Will now turn on STATION mode and commence network scan!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(3000);

  const auto noNetworks = WiFi.scanNetworks();
  ESP_LOGI(TAG, "Found %d networks", noNetworks);

  String result = "";

  for (auto i = 0; i < noNetworks; ++i)
  {
    result += "<option class=\"list-group-item\">";
    result += WiFi.SSID(i).c_str();
    result += "</option>";
  }

  return result;
}

static void startWifiSta(const String &ssid, const String &pass)
{
  const auto success = WiFi.softAPdisconnect();
  if (!success)
  {
    ESP_LOGE(TAG, "SoftAP couldn't be switched off");
  }

  wifi_config_t wifiConfig{};
  strcpy((char *)wifiConfig.sta.ssid, "WiFi-2.4");
  strcpy((char *)wifiConfig.sta.password, "180898Delia!");

  const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
  ESP_LOGI(TAG, "Credential set... Restarting");
  ESP.restart();
}

static AsyncWebServer createWebServer(const String &networkList)
{
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "Couldn't mount filesystem");
  }

  AsyncWebServer server(80);
  server.on("/", HTTP_GET,
            [&networkList](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /");
              request->send(SPIFFS, "/index.html", String(), false,
                            [&networkList](const String &var)
                            {
                              if (var == "NETWORKS")
                              {
                                return networkList;
                              }

                              return String();
                            });
            });
  server.on("/bootstrap.css", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /bootstrap.css");
              request->send(SPIFFS, "/bootstrap.css", "text/css");
            });
  server.on("/jquery.js", HTTP_GET,
            [](AsyncWebServerRequest *request)

            {
              ESP_LOGI(TAG, "Received GET request on /jquery.js");
              request->send(SPIFFS, "/jquery.js", "text/javascript");
            });
  server.on("/popper.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /popper.js");
              request->send(SPIFFS, "/popper.js", "text/javascript");
            });
  server.on("/bootstrap.js", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              ESP_LOGI(TAG, "Received GET request on /bootstrap.js");
              request->send(SPIFFS, "/bootstrap.js", "text/javascript");
            });
  server.on(
      "/wifi", HTTP_POST,
      [](AsyncWebServerRequest *request)
      {
        // do nothing
      },
      [](AsyncWebServerRequest *request, String filename, size_t index,
         uint8_t *data, size_t len, bool final)
      {
        // do nothing
      },
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total)
      {
        ESP_LOGI(TAG, "Received POST request on /wifi with body");
        request->send(200);
        ESP_LOGI(TAG, "Received data from website %s", data);

        const auto delimiter = " ";
        const auto ssid      = strtok((char *)data, delimiter);
        const auto pass      = strtok(nullptr, delimiter);

        startWifiSta(ssid, pass);
      });

  server.begin();

  ESP_LOGI(TAG, "Web Server is up!");

  return server;
}

static void startWifiAp()
{
  ESP_LOGI(
      TAG,
      "WiFi connection couldn't be established. Attempting to start Soft AP");

  const auto networkList = createNetworkList();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("sensor-1f2a46kg", "3599Andrei!");
  const auto apIp = WiFi.softAPIP().toString();
  ESP_LOGI(TAG, "Soft AP is up! IP: %s", apIp.c_str());

  const auto webServer = createWebServer(networkList);

  ESP_LOGI(TAG, "Web server is running");

  while (true)
  {
    delay(5000); // avoid destruction of the web server
                 // just wait for the credentials
  }
}

void NetUtils::startWifi()
{
  ESP_LOGI(TAG, "Attempting to connect to WiFi");

  if (WiFi.begin() == WL_CONNECT_FAILED)
  {
    startWifiAp();
  }

  ESP_LOGI(TAG, "Credentials are ok... waiting for WiFi to go up");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
  }

  ESP_LOGI(TAG, "Connection successful");
}

static bool checkExistingKey(const std::string &content)
{
  /* TODO add signature check
   format:
   key(hex) timestamp
   signature
   */

  if (content.empty())
  {
    return false;
  }

  return false;
}

static std::unique_ptr<uint8_t[]> performClientHello()
{
  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_HELLO_ENDPOINT;
  client.begin(endpoint);

  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  const auto statusCode = client.POST(
      "LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNKekNDQWM2Z0F3SUJBZ0lVRWV0TlN4"
      "cEEvanY0NlhYYnpVbmwySzJwMDYwd0NnWUlLb1pJemowRUF3SXcKYVRFTE1Ba0dBMVVFQmhN"
      "Q1VrOHhFREFPQmdOVkJBZ01CMUp2YldGdWFXRXhFakFRQmdOVkJBY01DVUoxWTJoaApjbVZ6"
      "ZERFS01BZ0dBMVVFQ2d3Qkx6RUtNQWdHQTFVRUN3d0JMekVLTUFnR0ExVUVBd3dCTHpFUU1B"
      "NEdDU3FHClNJYjNEUUVKQVJZQkx6QWVGdzB5TWpBME16QXhPRFV4TURCYUZ3MHlNekEwTWpV"
      "eE9EVXhNREJhTUdreEN6QUoKQmdOVkJBWVRBbEpQTVJBd0RnWURWUVFJREFkU2IyMWhibWxo"
      "TVJJd0VBWURWUVFIREFsQ2RXTm9ZWEpsYzNReApDakFJQmdOVkJBb01BUzh4Q2pBSUJnTlZC"
      "QXNNQVM4eENqQUlCZ05WQkFNTUFTOHhFREFPQmdrcWhraUc5dzBCCkNRRVdBUzh3V2pBVUJn"
      "Y3Foa2pPUFFJQkJna3JKQU1EQWdnQkFRY0RRZ0FFUzdEclF4TkRNeFVmeDdGL1NkZUQKSDFB"
      "OW50dEswbHJTRS92RHlOYW9DNFlnekM4MngvVy80cWdYL1NobVUzVnhvKzhobjd3czNxZ0RZ"
      "Z3d1S2lOeQo0S05UTUZFd0hRWURWUjBPQkJZRUZBVXJZaUs0S3NtMEpBbDlNR3pBZk9uQWM2"
      "SnRNQjhHQTFVZEl3UVlNQmFBCkZBVXJZaUs0S3NtMEpBbDlNR3pBZk9uQWM2SnRNQThHQTFV"
      "ZEV3RUIvd1FGTUFNQkFmOHdDZ1lJS29aSXpqMEUKQXdJRFJ3QXdSQUlnTEFwRG5wTHp1by9n"
      "REpwVXEyM1NvRVVrSndSd0Vnb0xWZGRWMnd1ekdjVUNJQ3hnSzBRWgpEMDZ3UDAvZkVTSzdR"
      "cU1jT3dvK1hUOXVhVlVhSW9YNWdEblUKLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQ==");

  ESP_LOGI(TAG, "Server responded %d", statusCode);
  size_t length = 0;
  auto   serverPoint =
      crypto::decodeBase64((uint8_t *)client.getString().c_str(), length);

  ESP_LOGI(TAG, "Server certificate has been decoded");

  return std::move(serverPoint);
}

static String performKeyExchange(mbedtls_ecdh_context &ecdhParams)
{
  HTTPClient client;

  ecdhParams = crypto::generateEcdhParams();

  ESP_LOGI(TAG, "DH params have been generated");
  uint8_t buffer[KEY_SIZE * 2 + 1];
  buffer[0] = 0x04;
  mbedtls_mpi_write_binary(&ecdhParams.Q.X, buffer + 1, KEY_SIZE);
  mbedtls_mpi_write_binary(&ecdhParams.Q.Y, buffer + KEY_SIZE + 1, KEY_SIZE);

  size_t     outputLength = 0;
  const auto encodedParams =
      crypto::encodeBase64(buffer, KEY_SIZE * 2 + 1, outputLength);
  ESP_LOGI(TAG, "Encoded DH params: %s", encodedParams.c_str());

  uint8_t privateKey[] = {
      0x30, 0x78, 0x02, 0x01, 0x01, 0x04, 0x20, 0xa0, 0xc4, 0x6b, 0x41, 0x54,
      0xa0, 0x14, 0x06, 0xe9, 0xff, 0xc3, 0x46, 0x95, 0x89, 0x4c, 0xca, 0x52,
      0x18, 0xcc, 0xd1, 0xd4, 0x7f, 0x53, 0x55, 0xc6, 0xe7, 0x12, 0x91, 0x0a,
      0xc0, 0xbd, 0x79, 0xa0, 0x0b, 0x06, 0x09, 0x2b, 0x24, 0x03, 0x03, 0x02,
      0x08, 0x01, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x4b, 0xb0,
      0xeb, 0x43, 0x13, 0x43, 0x33, 0x15, 0x1f, 0xc7, 0xb1, 0x7f, 0x49, 0xd7,
      0x83, 0x1f, 0x50, 0x3d, 0x9e, 0xdb, 0x4a, 0xd2, 0x5a, 0xd2, 0x13, 0xfb,
      0xc3, 0xc8, 0xd6, 0xa8, 0x0b, 0x86, 0x20, 0xcc, 0x2f, 0x36, 0xc7, 0xf5,
      0xbf, 0xe2, 0xa8, 0x17, 0xfd, 0x28, 0x66, 0x53, 0x75, 0x71, 0xa3, 0xef,
      0x21, 0x9f, 0xbc, 0x2c, 0xde, 0xa8, 0x03, 0x62, 0x0c, 0x2e, 0x2a, 0x23,
      0x72, 0xe0};
  size_t     signatureLength;
  const auto signature = crypto::signEcdsa(buffer, KEY_SIZE * 2 + 1,
                                           signatureLength, privateKey, 122);

  const auto signatureEncoded =
      crypto::encodeBase64(signature.get(), signatureLength, outputLength);

  String payload = "{\"publicParams\":\"" + encodedParams +
                   "\",\"signature\":\"" + signatureEncoded.c_str() + "\"}";
  ESP_LOGI(TAG, "Sending payload: %s", payload.c_str());

  const auto endpoint = ATTESTATION_SERVER + KEY_EXCHANGE_ENDPOINT;
  client.begin(endpoint);
  client.addHeader("Content-Type", "application/json");
  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());

  const auto statusCode = client.POST(payload.c_str());
  ESP_LOGI(TAG, "Server responded %d", statusCode);

  return client.getString();
}

static void performClientFinish(const char *publicParams, const char *signature,
                                const char *test, const uint8_t *serverPoint,
                                mbedtls_ecdh_context &ecdhParams)
{
  size_t     paramsLength = 0;
  const auto decodedPublicParams =
      crypto::decodeBase64((uint8_t *)publicParams, paramsLength);

  size_t     signatureLength = 0;
  const auto decodedSignature =
      crypto::decodeBase64((uint8_t *)signature, signatureLength);

  mbedtls_ecp_point point;
  mbedtls_ecp_point_init(&point);

  mbedtls_mpi_init(&point.X);
  mbedtls_mpi_init(&point.Y);
  mbedtls_mpi_init(&point.Z);

  uint8_t pt[KEY_SIZE] = "";
  memcpy(pt, serverPoint, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.X, pt, KEY_SIZE);

  memcpy(pt, serverPoint + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, pt, KEY_SIZE);

  mbedtls_mpi_lset(&point.Z, 1);

  auto signatureVerifies =
      crypto::verifyEcdsa(decodedPublicParams.get(), paramsLength,
                          decodedSignature.get(), signatureLength, point);

  if (!signatureVerifies)
  {
    ESP_LOGE(TAG, "Attestation server might be compromised");
    ESP.restart();
  }

  memcpy(pt, decodedPublicParams.get() + 1, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.X, pt, KEY_SIZE);

  memcpy(pt, decodedPublicParams.get() + 1 + KEY_SIZE, KEY_SIZE);
  mbedtls_mpi_read_binary(&point.Y, pt, KEY_SIZE);

  mbedtls_mpi_lset(&point.Z, 1);

  const auto generatedSecret = crypto::generateSharedSecret(ecdhParams, point);

  ESP_LOGI(TAG, "Test bytes: %s", test);

  size_t     testLength = 0;
  const auto testBytes  = crypto::decodeBase64((uint8_t *)test, testLength);

  const auto iv       = crypto::generateRandomSequence(KEY_SIZE / 2);
  size_t     ivLength = 0;
  const auto ivBase64 = crypto::encodeBase64(iv.get(), KEY_SIZE / 2, ivLength);

  ESP_LOGI(TAG, "IV: %s", ivBase64.c_str());

  uint8_t payload[KEY_SIZE] = "";
  memcpy(payload, iv.get(),
         KEY_SIZE / 2); // before encryption since the IV is mutated!

  size_t     outputLength2 = 0;
  const auto keyEncoded =
      crypto::encodeBase64(generatedSecret.get(), 32, outputLength2);
  ESP_LOGI(TAG, "Key: %s", keyEncoded.c_str());

  uint16_t   cipherTextSize = 0;
  const auto cipherText     = crypto::encryptAes(
          testBytes.get(), generatedSecret.get(), iv.get(), cipherTextSize);

  memcpy(payload + KEY_SIZE / 2, cipherText.get(), KEY_SIZE / 2);

  size_t     outputLength = 0;
  const auto payloadEncoded =
      crypto::encodeBase64(payload, KEY_SIZE, outputLength);

  HTTPClient client;
  const auto endpoint = ATTESTATION_SERVER + CLIENT_FINISHED_ENDPOINT;
  client.begin(endpoint);
  client.addHeader("Content-Type", "text/plain");
  ESP_LOGI(TAG, "Post to %s", endpoint.c_str());
  ESP_LOGI(TAG, "Post data: %s", payloadEncoded.c_str());

  const auto statusCode = client.POST(payloadEncoded);
  ESP_LOGI(TAG, "Server responded %d", statusCode);

  // TODO persist it to storage
  memcpy(MQTT_PSK_KEY, generatedSecret.get(), KEY_SIZE);
}

static void performAttestationProcess()
{
  ESP_LOGI(TAG, "Starting the attestation process");

  const auto serverPoint = performClientHello();

  mbedtls_ecdh_context ecdhParams;
  const auto           serverPayload = performKeyExchange(ecdhParams);

  auto       *root = cJSON_Parse(serverPayload.c_str());
  const auto *publicParams =
      cJSON_GetObjectItem(root, "publicParams")->valuestring;
  const auto *signature = cJSON_GetObjectItem(root, "signature")->valuestring;
  const auto *test      = cJSON_GetObjectItem(root, "test")->valuestring;

  performClientFinish(publicParams, signature, test, serverPoint.get(),
                      ecdhParams);
}

static bool checkAttestationStatus()
{
  auto       spiffs  = SpiffsUtils::getInstance();
  const auto content = spiffs->readText("/secret.key");

  return checkExistingKey(content);
}

void NetUtils::attestDevice()
{
  // TODO add rtos task to check key each hour

  ESP_LOGI(TAG, "Checking attestation status");

  if (checkAttestationStatus())
  {
    return;
  }

  performAttestationProcess();
}

static void handleFailure(const char *message)
{
  ESP_LOGE(TAG, "%s", message);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void registerMqttHandlers(esp_mqtt_client_handle_t client)
{
  esp_mqtt_client_register_event(
      client, MQTT_EVENT_CONNECTED,
      [](void *handlerArgs, esp_event_base_t base, int32_t eventId,
         void *eventData) {
        ESP_LOGI("MQTT_INFO", "Connection to broker established successfully");
      },
      client);
  esp_mqtt_client_register_event(
      client, MQTT_EVENT_DISCONNECTED,
      [](void *handlerArgs, esp_event_base_t base, int32_t eventId,
         void *eventData)
      {
        ESP_LOGI("MQTT_INFO",
                 "Disconnected from broker... waiting 30 seconds before "
                 "attempting to reinitialize client connection");

        vTaskDelay(30000 / portTICK_PERIOD_MS);
        esp_mqtt_client_start(((esp_mqtt_event_handle_t)eventData)->client);
      },
      client);
  esp_mqtt_client_register_event(
      client, MQTT_EVENT_PUBLISHED,
      [](void *handlerArgs, esp_event_base_t base, int32_t eventId,
         void *eventData)
      {
        ESP_LOGI("MQTT_INFO", "Message %d has been successfully published",
                 ((esp_mqtt_event_handle_t)eventData)->msg_id);
      },
      client);
  esp_mqtt_client_register_event(
      client, MQTT_EVENT_ERROR,
      [](void *handlerArgs, esp_event_base_t base, int32_t eventId,
         void *eventData)
      {
        const auto *event =
            reinterpret_cast<esp_mqtt_event_handle_t>(eventData);
        static const auto *TAG = "MQTT_INFO";

        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
          ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x",
                   event->error_handle->esp_tls_last_esp_err);
          ESP_LOGI(TAG, "Last tls stack error number: 0x%x",
                   event->error_handle->esp_tls_stack_err);
          ESP_LOGI(TAG, "Last captured errno : %d (%s)",
                   event->error_handle->esp_transport_sock_errno,
                   strerror(event->error_handle->esp_transport_sock_errno));
        }
        else if (event->error_handle->error_type ==
                 MQTT_ERROR_TYPE_CONNECTION_REFUSED)
        {
          ESP_LOGI(TAG, "Connection refused error: 0x%x",
                   event->error_handle->connect_return_code);
        }
        else
        {
          ESP_LOGW(TAG, "Unknown error type: 0x%x",
                   event->error_handle->error_type);
        }
      },
      client);
}

esp_mqtt_client_handle_t NetUtils::initMqttConnection()
{
  ESP_LOGI(TAG, "Initializing mqtt connection...");

  static const auto    *hint = "node1";
  static psk_hint_key_t pskConf{MQTT_PSK_KEY, KEY_SIZE, hint};

  ESP_LOGI(TAG, "Mqtt psk hint: %s", hint);

  esp_mqtt_client_config_t mqttCfg{};
  mqttCfg.uri          = MQTT_SERVER;
  mqttCfg.client_id    = "node1";
  mqttCfg.psk_hint_key = &pskConf;

  auto client = esp_mqtt_client_init(&mqttCfg);
  if (!client)
  {
    handleFailure("Client not acquired... Will restart in 5 seconds");
  }

  registerMqttHandlers(client);
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));

  return client;
}

static void publishCapability(esp_mqtt_client_handle_t &client,
                              SensorType                capability)
{
  const auto *topic      = "data";
  const auto *data       = "{\"data\":%d,\"origin\":\"node1\"}";
  char        buffer[64] = "";
  sprintf(buffer, data, SensorUtils::querySensor(capability));

  const auto returnCode =
      esp_mqtt_client_publish(client, topic, data, strlen(data), 0, 0);
  ESP_LOGI(TAG, "Message on topic %s has mid: %d", topic, returnCode);
}

void NetUtils::publishAll(esp_mqtt_client_handle_t &client)
{
  ESP_LOGI(TAG, "Publishing all data to the broker");

  for (auto capability : capabilities)
  {
    publishCapability(client, capability);
  }
}