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

  /* const auto plaintext = "hello";
  uint8_t    key[]     = {
      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2,
      0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5,
      0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
  };

  uint8_t    iv1[]      = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                   0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
  uint16_t   outputSize = 0;
  const auto cipherText =
      crypto::encryptAes((uint8_t *)plaintext, key, iv1, outputSize);

  uint8_t    iv2[]    = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                   0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
  const auto restored = crypto::decryptAes(cipherText.get(), 16, key, iv2);

  ESP_LOGI(TAG, "Restored: %s", restored.get()); */

  const auto dhParams = crypto::generateEcdhParams();
  ESP_LOG_BUFFER_HEX(TAG, dhParams.get(), 32);
}