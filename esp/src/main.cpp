#include "esp_log.h"
#include "net_utils.h"

static constexpr const char *TAG = "MAIN";

extern "C" void app_main()
{
  ESP_LOGI(TAG, "Starting node...");

  const auto netUtils = NetUtils::getInstance();
  netUtils->startWifi();

  while (true)
  {
    ESP_LOGI(TAG, "Looping...");
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
  }
}