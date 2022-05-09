#ifndef _ML_UTILS_H
#define _ML_UTILS_H

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

class MlPredictor
{
public:
  explicit MlPredictor(const uint8_t *modelBytes);

  float *predict();

  float *getInputBuffer();

private:
  tflite::MicroInterpreter interpreter;
};

#endif // _ML_UTILS_H