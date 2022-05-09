#include "ml_utils.h"

#include "esp_log.h"

#define TENSOR_ARENA_SIZE 2048

static tflite::MicroErrorReporter errorReporter;
static tflite::AllOpsResolver     resolver;
static constexpr auto            *TAG = "ML";
static uint8_t                    tensorArena[TENSOR_ARENA_SIZE];

MlPredictor::MlPredictor(const uint8_t *modelBytes)
    : interpreter(tflite::GetModel(modelBytes), resolver, tensorArena,
                  TENSOR_ARENA_SIZE, &errorReporter)
{
  if (interpreter.AllocateTensors() != kTfLiteOk)
  {
    ESP_LOGE(TAG, "Tensors could not be allocated");
  }

  const auto arenaUsedBytes = interpreter.arena_used_bytes();
  ESP_LOGI(TAG, "Arena used size: %d", arenaUsedBytes);
}

float *MlPredictor::getInputBuffer()
{
  return interpreter.input(0)->data.f;
}

float *MlPredictor::predict()
{
  ESP_LOGI(TAG, "Calling ML predict");

  if (interpreter.Invoke() != kTfLiteOk)
  {
    ESP_LOGE(TAG, "Invocation failed");
  }

  return interpreter.output(0)->data.f;
}