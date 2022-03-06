#include "spiffs_utils.h"

#include <cstdio>

#include "esp_log.h"
#include "esp_spiffs.h"

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

std::string SpiffsUtils::readText(const std::string &path)
{
  const auto fileDeleter = [](FILE *f) { fclose(f); };
  std::unique_ptr<FILE, decltype(fileDeleter)> file(fopen(path.c_str(), "r"),
                                                    fileDeleter);

  if (!file.get())
  {
    ESP_LOGE(TAG, "Couldn't read from %d", path);

    return "";
  }

  fseek(file.get(), 0, SEEK_END);
  const auto fileSize = ftell(file.get());
  fseek(file.get(), 0, SEEK_SET);

  std::unique_ptr<char[]> buffer(new char[fileSize + 1]);
  fread(buffer.get(), sizeof(char), fileSize, file.get());

  return std::string(buffer.get());
}

SpiffsUtils::SpiffsUtils()
{
  ESP_LOGI(TAG, "Initializing SPIFF");

  esp_vfs_spiffs_conf_t config{};
  config.base_path              = "/";
  config.partition_label        = nullptr;
  config.max_files              = 3;
  config.format_if_mount_failed = true;

  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&config));
}