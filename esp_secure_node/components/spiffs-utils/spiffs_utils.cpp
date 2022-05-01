#include "spiffs_utils.h"

#include "SPIFFS.h"
#include "esp_log.h"
#include <cstdio>

const char                  *SpiffsUtils::TAG = "SPIFFS_UTILS";
std::shared_ptr<SpiffsUtils> SpiffsUtils::instance =
    std::shared_ptr<SpiffsUtils>(nullptr);

std::shared_ptr<SpiffsUtils> SpiffsUtils::getInstance()
{
  if (instance.get() == nullptr)
  {
    instance.reset(new SpiffsUtils());
  }

  return instance;
}

String SpiffsUtils::readText(const String &path)
{
  ESP_LOGI(TAG, "Reading from %s", path.c_str());
  auto file = SPIFFS.open(path, FILE_READ);
  if (!file)
  {
    ESP_LOGE(TAG, "Couldn't read from %s", path.c_str());

    return "";
  }

  const auto content = file.readString();
  file.close();

  return content;
}

void SpiffsUtils::writeText(const String &path, const String &content)
{
  ESP_LOGI(TAG, "Writing to %s", path.c_str());
  auto file = SPIFFS.open(path, FILE_WRITE);
  if (!file)
  {
    ESP_LOGE(TAG, "Couldn't write to %s", path.c_str());
  }

  const auto wrote = file.write((uint8_t *)content.c_str(), content.length());
  if (content.length() != wrote)
  {
    ESP_LOGE(TAG, "Not all bytes have been written");
  }

  file.close();
}

SpiffsUtils::SpiffsUtils() {}