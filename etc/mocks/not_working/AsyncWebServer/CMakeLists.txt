idf_component_register(
  SRCS 
    "AsyncEventSource.cpp"
    "AsyncWebSocket.cpp"
    "SPIFFSEditor.cpp"
    "WebAuthentication.cpp"
    "WebHandlers.cpp"
    "WebResponses.cpp"
    "WebServer.cpp"
  INCLUDE_DIRS "."
  REQUIRES arduino AsyncTCP mbedtls spiffs)