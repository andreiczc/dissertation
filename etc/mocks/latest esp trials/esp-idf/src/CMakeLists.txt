# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources})
target_add_binary_data(${COMPONENT_TARGET} "../data/device.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../data/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../data/wallet.key" TEXT)