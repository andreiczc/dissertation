#include "coap_client.h"
#include "crypto_utils.h"
#include "esp_event.h"
#include "esp_log.h"
#include "net_utils.h"
#include "nvs_flash.h"

static constexpr const char *TAG = "MAIN";

static void init()
{
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

extern "C" void app_main()
{
  ESP_LOGI(TAG, "Starting node...");
  init();
  ESP_LOGI(TAG, "Init complete");

  /* const auto netUtils = NetUtils::getInstance();
  netUtils->startWifi();

  // const auto client = netUtils->initMqttConnection();
  CoapClient client("192.168.0.180", 5683);

  size_t payloadLength;
  auto  *payload = client.doGet(payloadLength, "test"); */

  const auto encoded = crypto::encodeBase64((uint8_t *)"hello");
  ESP_LOGI(TAG, "Encoded: %s", encoded.get());

  const auto decoded = crypto::decodeBase64(encoded.get());
  ESP_LOGI(TAG, "Decoded: %s", decoded.get());
}