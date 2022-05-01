idf_component_register(
  SRCS 
      "TagReader.cpp"
  INCLUDE_DIRS "."
  REQUIRES arduino)

set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized)
