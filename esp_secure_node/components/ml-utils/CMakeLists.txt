idf_component_register(
  SRCS "ml_utils.cpp" "model_data.cpp"
  INCLUDE_DIRS "."
  REQUIRES 
          tflite-lib)
