#include "net_utils.h"

#include <AsyncTCP.h>

#include "Arduino.h"
#include "WiFi.h"
#include "attestation_utils.h"
#include "cJSON.h"
#include "cred_server.h"
#include "crypto_utils.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "ml_utils.h"
#include "model_data.h"
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
  bool ml;
};

// TODO add certificate checks in esp and server

// CONSTANTS
static constexpr auto *TAG         = "NET";
static constexpr auto *MQTT_SERVER = "mqtts://130.162.253.10:8883";
static constexpr auto  OBJECT_ID   = 1001;
static constexpr auto  INSTANCE_ID = 0;

static std::unique_ptr<uint8_t[]> MQTT_PSK_KEY(nullptr);
static MlPredictor                predictor(ml::model::modelBytes);

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

void NetUtils::startWifi()
{
  if (!SPIFFS.begin())
  {
    ESP_LOGE(TAG, "Couldn't mount filesystem");
  }

  ESP_LOGI(TAG, "Attempting to connect to WiFi");

  if (WiFi.begin() == WL_CONNECT_FAILED)
  {
    credentials::startCredentialsServer();
  }

  ESP_LOGI(TAG, "Credentials are ok... waiting for WiFi to go up");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
  }

  ESP_LOGI(TAG, "Connection successful");
}

static void performAttestationProcess()
{
  ESP_LOGI(TAG, "Starting the attestation process");

  const auto serverPoint = attestation::performClientHello();

  mbedtls_ecdh_context ecdhParams;
  const auto serverPayload = attestation::performKeyExchange(ecdhParams);

  auto       *root = cJSON_Parse(serverPayload.c_str());
  const auto *publicParams =
      cJSON_GetObjectItem(root, "publicParams")->valuestring;
  const auto *signature = cJSON_GetObjectItem(root, "signature")->valuestring;
  const auto *test      = cJSON_GetObjectItem(root, "test")->valuestring;

  MQTT_PSK_KEY = attestation::performClientFinish(
      publicParams, signature, test, serverPoint.get(), ecdhParams);
}

static bool checkAttestationStatus()
{
  auto       spiffs  = SpiffsUtils::getInstance();
  const auto content = spiffs->readText("/secret.key");

  return attestation::checkExistingKey(content);
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
  static psk_hint_key_t pskConf{MQTT_PSK_KEY.get(), KEY_SIZE, hint};

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

  if (settings.ml)
  {
    ESP_LOGI(TAG, "Performing prediction for %s", name.c_str());
  }

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

    const auto *mlItem = cJSON_GetObjectItem(currItem, "ml");
    const auto  ml     = !cJSON_IsFalse(mlItem);

    // deep copy
    auto &setting   = capabilitiesSettings.at(name);
    setting.enabled = enabledValue;
    strcpy(setting.blockchain, blockchain);
    setting.ml = ml;
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
    auto *ml         = cJSON_CreateBool(setting.ml);

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
                 sprintf(buffer,
                         "{\"enabled\": %s, \"blockchain\":\"%s\", \"ml\":%s}",
                         currentSettings.enabled ? "true" : "false",
                         currentSettings.blockchain,
                         currentSettings.ml ? "true" : "false");

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
          const auto *mlItem = cJSON_GetObjectItem(root, "ml");
          const auto  ml     = !cJSON_IsFalse(mlItem);

          // deep copy
          auto &setting   = capabilitiesSettings.at(name);
          setting.enabled = enabled;
          strcpy(setting.blockchain, blockchain);
          setting.ml = ml;

          ESP_LOGI(
              TAG,
              "Altered setting for %s. Enabled: %d; Blockchain: %s, ML: %d",
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