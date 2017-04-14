# need 3.0 for multiple line strings
#cmake_minimum_required(VERSION 3.0)

#set(SGX_SDK /opt/intel/sgxsdk)
#set(SGX_ARCH x64)
#set(SGX_MODE SIM) # SGX mode: sim, hw
# build mode: debug OR prerelease (NOT BOTH)
#set(SGX_DEBUG TRUE)
#set(SGX_PRERELEASE FALSE)

#include (../../cmake/Init.cmake)
#include (../../cmake/ConfigSGX.cmake)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# TODO: see if possible to conditionally run above commands based on the mode (e.g. being added or using as the root CMakeLists.txt)

# TODO: convert to absolute path using this: https://stackoverflow.com/questions/39027269/cmake-convert-relative-path-to-absolute-path-with-build-directory-as-current-d
set(MBEDTLS_SGX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")

add_subdirectory(App)
add_subdirectory(Enclave)
