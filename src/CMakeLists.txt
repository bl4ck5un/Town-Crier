# need 3.0 for multiple line strings
cmake_minimum_required(VERSION 3.0)
project(Town-Crier)

set(SGX_SDK /opt/intel/sgxsdk)
set(SGX_ARCH x64)
set(SGX_MODE SIM) # SGX mode: sim, hw
# build mode: debug OR prerelease (NOT BOTH)
set(SGX_DEBUG TRUE)
set(SGX_PRERELEASE FALSE)

include (cmake/Init.cmake)
include (cmake/ConfigSGX.cmake)

set(TC_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(MBEDTLS_LIB_DIR ${TC_ROOT_DIR}/Enclave/mbedtls-SGX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(App)
add_subdirectory(Enclave)
