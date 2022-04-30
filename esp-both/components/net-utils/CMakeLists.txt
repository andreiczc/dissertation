idf_component_register(
  SRCS "net_utils.cpp"
  INCLUDE_DIRS "."
  REQUIRES 
          mbedtls 
          arduino 
          AsyncWebServer 
          crypto-utils 
          mqtt 
          json 
          spiffs-utils)
