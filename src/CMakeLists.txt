# need 3.0 for multiple line strings
cmake_minimum_required(VERSION 3.0)
project(Town-Crier)

set(SGX_SDK /opt/intel/sgxsdk)
set(SGX_ARCH x64)
set(SGX_MODE SIM CACHE STRING "SGX mode: SIM or HW") # SGX mode: sim, hw
set(SGX_BUILD DEBUG CACHE STRING "Build: DEBUG, PRERELEASE or RELEASE")

include (cmake/Init.cmake)
include (cmake/ConfigSGX.cmake)

set(TC_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(MBEDTLS_LIB_DIR ${TC_ROOT_DIR}/Enclave/mbedtls-SGX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX /opt)

add_subdirectory(App)
add_subdirectory(Enclave)