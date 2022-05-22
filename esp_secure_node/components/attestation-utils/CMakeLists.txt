idf_component_register(
  SRCS "attestation_utils.cpp"
  INCLUDE_DIRS "."
  REQUIRES 
          arduino
          crypto-utils
          mbedtls
          spiffs-utils
          json)
