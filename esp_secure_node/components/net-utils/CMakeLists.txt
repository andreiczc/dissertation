idf_component_register(
  SRCS "net_utils.cpp"
  INCLUDE_DIRS "."
  REQUIRES 
          mbedtls 
          arduino  
          crypto-utils 
          ml-utils
          mqtt 
          json 
          spiffs-utils
          sensor-utils
          web3-client
          cbor-ipso
          AsyncWebServer)
