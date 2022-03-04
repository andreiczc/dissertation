#include "net_utils.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <cstring>

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
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
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