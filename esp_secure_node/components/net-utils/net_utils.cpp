#include "net_utils.h"

#include <AsyncTCP.h>

#include "Arduino.h"
#include "WiFi.h"
#include "cJSON.h"
#include "crypto_utils.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "sensor_utils.h"
#include "smart_obj.h"
#include "spiffs_utils.h"
#include "web3_client.h"
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <map>
#include <vector>

#define KEY_SIZE 32

struct SensorSetting
{
  bool enabled;
  char blockchain[512];
  char ml[256];
};

// TODO add certificate checks in esp and server

// CONSTANTS
static constexpr auto *TAG         = "NET";
static constexpr auto *MQTT_SERVER = "mqtts://130.162.253.10:8883";
static constexpr auto  OBJECT_ID   = 1001;
static constexpr auto  INSTANCE_ID = 0;
static const String    ATTESTATION_SERVER =
    "http://130.162.253.10:8080/attestation/";
static const String DEVICE_CERT_PATH         = "/device.crt";
static const String DEVICE_KEY_PATH          = "/device.key";
static const String CLIENT_HELLO_ENDPOINT    = "clientHello";
static const String KEY_EXCHANGE_ENDPOINT    = "keyExchange";
static const String CLIENT_FINISHED_ENDPOINT = "clientFinished";

static const std::map<SensorType, int> resourceMap{
    {SensorType::TEMPERATURE, 5001},
    {SensorType::HUMIDITY, 5002},
    {SensorType::GAS, 5003},
    {SensorType::VIBRATION, 5004}};
static const std::vector<SensorType> capabilities{
    SensorType::TEMPERATURE, SensorType::HUMIDITY, SensorType::GAS,
    SensorType::VIBRATION};
static const std::map<SensorType, String> capabilityName{
    {SensorType::TEMPERATURE, "temp"},
    {SensorType::HUMIDITY, "humidity"},
    {SensorType::GAS, "gas"},
    {SensorType::VIBRATION, "vibration"}};
static std::map<String, SensorSetting> capabilitiesSettings{
    {"temp", SensorSetting{true, "", ""}},
    {"humidity", SensorSetting{true, "", ""}},
    {"gas", SensorSetting{true, "", ""}},
    {"vibration", SensorSetting{true, "", ""}}};

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
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "Couldn't mount filesystem");
  }

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

static bool checkExistingKey(const String &content)
{
  /* TODO add signature check
   format:
   key(hex) timestamp
   signature
   */

  if (content.isEmpty())
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
  const auto deviceCertificate =
      SpiffsUtils::getInstance()->readText(DEVICE_CERT_PATH);
  const auto statusCode = client.POST(deviceCertificate.c_str());

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

  const auto privateKeyBase64 =
      SpiffsUtils::getInstance()->readText(DEVICE_KEY_PATH);
  size_t     keyLength = 0;
  const auto privateKey =
      crypto::decodeBase64((uint8_t *)privateKeyBase64.c_str(), keyLength);
  size_t     signatureLength;
  const auto signature = crypto::signEcdsa(
      buffer, KEY_SIZE * 2 + 1, signatureLength, privateKey.get(), 122);

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
  const auto name     = capabilityName.at(capability);
  auto       settings = capabilitiesSettings.at(name);

  ESP_LOGI(TAG, "Publishing %s", name.c_str());

  if (!settings.enabled)
  {
    return;
  }

  const auto        sensorValue = SensorUtils::querySensor(capability);
  ipso::SmartObject smartObj;
  smartObj.addValue(ipso::SmartObjectValue(
      OBJECT_ID, INSTANCE_ID, resourceMap.at(capability), sensorValue));
  const auto stringValue = smartObj.cbor();

  char topic[64] = "";
  sprintf(topic, "%d/%d/%d", OBJECT_ID, INSTANCE_ID,
          resourceMap.at(capability));

  const auto returnCode = esp_mqtt_client_publish(
      client, topic, stringValue.c_str(), stringValue.length(), 0, 0);
  ESP_LOGI(TAG, "Message on topic %s has mid: %d", topic, returnCode);

  if (strlen(settings.blockchain))
  {
    const auto *contractAddress = strtok(settings.blockchain, " ");
    const auto *payload         = strtok(nullptr, " ");

    const auto              bufferSize = strlen(payload) + 64;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);

    char encodedValue[65] = "";
    encodedValue[64]      = 0;
    memset(encodedValue, '0', 56);

    char valueBuffer[9] = "";
    sprintf(valueBuffer, "%08x", sensorValue);
    memcpy(encodedValue + 56, valueBuffer, 8);

    sprintf(buffer.get(), payload, encodedValue);

    ESP_LOGI(TAG, "Encoded value: %s", encodedValue);
    ESP_LOGI(TAG, "Formatter used: %s", payload);
    ESP_LOGI(TAG, "Calling contract %s with data %s for %s", contractAddress,
             buffer.get(), name.c_str());

    std::string formattedPayload(buffer.get());
    blockchain::callContract(contractAddress, formattedPayload);
  }
}

void NetUtils::publishAll(esp_mqtt_client_handle_t &client)
{
  ESP_LOGI(TAG, "Publishing all data to the broker");

  for (auto capability : capabilities)
  {
    publishCapability(client, capability);
  }
}

static void loadSettingsFromFlash()
{
  ESP_LOGI(TAG, "Reading settings from flash");

  const auto spiffsUtils = SpiffsUtils::getInstance();
  const auto settings    = spiffsUtils->readText("/settings.json");

  const auto *root = cJSON_Parse((char *)settings.c_str());

  for (auto capability : capabilities)
  {
    const auto name = capabilityName.at(capability);

    const auto *currItem    = cJSON_GetObjectItem(root, name.c_str());
    const auto *enabledItem = cJSON_GetObjectItem(currItem, "enabled");

    const auto enabledValue = !cJSON_IsFalse(enabledItem);
    const auto blockchain =
        cJSON_GetObjectItem(currItem, "blockchain")->valuestring;
    const auto ml = cJSON_GetObjectItem(currItem, "ml")->valuestring;

    // deep copy
    auto &setting   = capabilitiesSettings.at(name);
    setting.enabled = enabledValue;
    strcpy(setting.blockchain, blockchain);
    strcpy(setting.ml, ml);
  }
}

static void writeSettingsToFlash()
{
  ESP_LOGI(TAG, "Writting settings to flash");

  auto *json = cJSON_CreateObject();

  for (const auto capability : capabilities)
  {
    const auto name    = capabilityName.at(capability);
    const auto setting = capabilitiesSettings.at(name);

    auto *item = cJSON_CreateObject();

    auto *enabled    = cJSON_CreateBool(setting.enabled);
    auto *blockchain = cJSON_CreateString(setting.blockchain);
    auto *ml         = cJSON_CreateString(setting.ml);

    cJSON_AddItemToObject(item, "enabled", enabled);
    cJSON_AddItemToObject(item, "blockchain", blockchain);
    cJSON_AddItemToObject(item, "ml", ml);

    cJSON_AddItemToObject(json, name.c_str(), item);
  }

  const auto string = cJSON_Print(json);
  cJSON_Delete(json);

  const auto spiffsUtils = SpiffsUtils::getInstance();
  spiffsUtils->writeText("/settings.json", string);
}

std::unique_ptr<AsyncWebServer> NetUtils::startManagementServer()
{
  loadSettingsFromFlash();

  std::unique_ptr<AsyncWebServer> server(new AsyncWebServer(80));
  server->on("/", HTTP_GET,
             [](AsyncWebServerRequest *request)
             {
               ESP_LOGI(TAG, "Received GET request on /");
               request->send(SPIFFS, "/config_index.html", String(), false,
                             [](const String &var)
                             {
                               if (var == "IP")
                               {
                                 return WiFi.localIP().toString();
                               }

                               return String();
                             });
             });

  for (auto capability : capabilities)
  {
    const auto   name     = capabilityName.at(capability);
    const String endpoint = String("/") + name;

    ESP_LOGI(TAG, "Adding endpoints for %s", endpoint.c_str());

    server->on(endpoint.c_str(), HTTP_GET,
               [](AsyncWebServerRequest *request)
               {
                 const auto endpoint = request->url();
                 const auto name     = endpoint.substring(1);
                 ESP_LOGI(TAG, "Received GET request on %s", endpoint.c_str());

                 const auto currentSettings = capabilitiesSettings.at(name);

                 char buffer[820];
                 sprintf(
                     buffer,
                     "{\"enabled\": %s, \"blockchain\":\"%s\", \"ml\":\"%s\"}",
                     currentSettings.enabled ? "true" : "false",
                     currentSettings.blockchain, currentSettings.ml);

                 request->send(200, String(), String(buffer));
               });

    server->on(
        endpoint.c_str(), HTTP_POST,
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
          const auto endpoint = request->url();
          const auto name     = endpoint.substring(1);

          ESP_LOGI(TAG, "Received POST request on %s", endpoint.c_str());
          ESP_LOGI(TAG, "Data: %s", data);

          auto       *root        = cJSON_Parse((char *)data);
          const auto *enabledItem = cJSON_GetObjectItem(root, "enabled");
          const auto  enabled     = !cJSON_IsFalse(enabledItem);

          const auto blockchain =
              cJSON_GetObjectItem(root, "blockchain")->valuestring;
          const auto ml = cJSON_GetObjectItem(root, "ml")->valuestring;

          // deep copy
          auto &setting   = capabilitiesSettings.at(name);
          setting.enabled = enabled;
          strcpy(setting.blockchain, blockchain);
          strcpy(setting.ml, ml);

          ESP_LOGI(
              TAG,
              "Altered setting for %s. Enabled: %d; Blockchain: %s, ML: %s",
              name.c_str(), setting.enabled, setting.blockchain, setting.ml);

          request->send(200);
          writeSettingsToFlash();
        });
  }

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server->begin();

  ESP_LOGI(TAG, "Management Web Server is up! IP: %s",
           WiFi.localIP().toString().c_str());

  return std::move(server);
}