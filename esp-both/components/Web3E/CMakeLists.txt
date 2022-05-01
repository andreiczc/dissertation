idf_component_register(
  SRCS  "Contract.cpp" 
        "Crypto.cpp" 
        "KeyID.cpp"
        "ScriptClient.cpp" 
        "TcpBridge.cpp" 
        "UdpBridge.cpp" 
        "Util.cpp" 
        "Web3.cpp"
  INCLUDE_DIRS "."
  REQUIRES 
      arduino
      Trezor
      TagReader
      uint256
      json)
