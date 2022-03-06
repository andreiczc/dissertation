#include "net_utils.h"

#include <cstring>

#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"

extern "C" const uint8_t caPemStart[] asm("_binary_ca_pem_start");
extern "C" const uint8_t caPemEnd[] asm("_binary_ca_pem_end");

static constexpr const int MAX_RETRIES        = 10;
static constexpr const int WIFI_CONNECTED_BIT = 1;
static constexpr const int WIFI_FAIL_BIT      = 2;
static int                 numRetries         = 0;

const char               *NetUtils::TAG = "NET_UTILS";
std::shared_ptr<NetUtils> NetUtils::instance =
    std::shared_ptr<NetUtils>(nullptr);

std::shared_ptr<NetUtils> NetUtils::getInstance()
{
  if (NetUtils::instance.get() == nullptr)
  {
    NetUtils::instance = std::shared_ptr<NetUtils>(new NetUtils());
  }

  return NetUtils::instance;
}

NetUtils::NetUtils()
{
  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_sta();
  const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

EventGroupHandle_t NetUtils::getWifiEventGroup() const
{
  return this->wifiEventGroup;
}

void NetUtils::startWiFiSta()
{
  this->wifiEventGroup = xEventGroupCreate();

  esp_event_handler_instance_t instanceWifiStart;
  esp_event_handler_instance_t instanceWifiDisconnected;
  esp_event_handler_instance_t instanceGotIp;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, WIFI_EVENT_STA_START,
      [](void *arg, esp_event_base_t event_base, int32_t event_id,
         void *event_data)
      {
        ESP_LOGI(TAG, "WiFi start event received...");
        esp_wifi_connect();
      },
      nullptr, &instanceWifiStart));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
      [](void *arg, esp_event_base_t event_base, int32_t event_id,
         void *event_data)
      {
        if (numRetries++ < MAX_RETRIES)
        {
          ESP_LOGI(TAG, "Trying to reconnect...");
          esp_wifi_connect();
        }
        else
        {
          numRetries = 0;
          ESP_LOGI(TAG, "Maximum number of retries excedeed");
          xEventGroupSetBits(NetUtils::getInstance()->getWifiEventGroup(),
                             WIFI_FAIL_BIT);
        }
      },
      nullptr, &instanceWifiDisconnected));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP,
      [](void *arg, esp_event_base_t event_base, int32_t event_id,
         void *event_data)
      {
        numRetries        = 0;
        const auto *event = static_cast<ip_event_got_ip_t *>(event_data);
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(NetUtils::getInstance()->getWifiEventGroup(),
                           WIFI_CONNECTED_BIT);
      },
      nullptr, &instanceGotIp));

  wifi_config_t wifiConfig{};
  strcpy((char *)wifiConfig.sta.ssid, "WiFi-2.4");
  strcpy((char *)wifiConfig.sta.password, "180898Delia!");
  wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_WPA3_PSK;
  wifiConfig.sta.pmf_cfg.capable    = true;
  wifiConfig.sta.pmf_cfg.required   = false;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Waiting for group bits to be set");
  auto bits = xEventGroupWaitBits(this->wifiEventGroup,
                                  WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
                                  pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(TAG, "Connected to AP with SSID %s", wifiConfig.sta.ssid);
  }
  else
  {
    ESP_LOGI(TAG, "Failed to connect... Will switch to AP mode now");
    // TODO create AP mode implementation
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      WIFI_EVENT, WIFI_EVENT_STA_START, &instanceWifiStart));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &instanceWifiDisconnected));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &instanceGotIp));

  vEventGroupDelete(this->wifiEventGroup);
}

void NetUtils::startWifi()
{
  ESP_LOGI(TAG, "Attempting to connect to WiFi...");

  startWiFiSta();
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

  esp_mqtt_client_config_t mqttCfg{};
  mqttCfg.uri      = "mqtts://172.23.109.3:8883";
  mqttCfg.cert_pem = reinterpret_cast<const char *>(caPemStart);

  auto client = esp_mqtt_client_init(&mqttCfg);
  if (!client)
  {
    ESP_LOGE(TAG, "Client not acquired... Will restart in 5 seconds");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_restart();
  }

  registerMqttHandlers(client);
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));

  return client;
}