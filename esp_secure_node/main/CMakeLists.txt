idf_component_register(
  SRCS "app.cpp"
  INCLUDE_DIRS ""
  REQUIRES 
    net-utils)

spiffs_create_partition_image(storage "../data" FLASH_IN_PROJECT)