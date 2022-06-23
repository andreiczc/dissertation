idf_component_register(
  SRCS  "DHT.cpp"
        "DHT_U.cpp"
  INCLUDE_DIRS "."
  REQUIRES
          arduino
          adafruit_unified)
