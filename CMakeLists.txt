cmake_minimum_required(VERSION 3.0)
project(Town-Crier)

# build options
option(BUILD_TESTS "build tc-test target" on)

# only support x64
set(SGX_ARCH x64)

# set SDK path
if (DEFINED ENV{SGX_SDK})
    set(SGX_SDK $ENV{SGX_SDK})
else()
    set(SGX_SDK /opt/intel/sgxsdk)
endif()

include(cmake/Init.cmake)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SGX_MODE SIM)
    set(SGX_BUILD DEBUG)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Prerelease)
    set(SGX_MODE HW)
    set(SGX_BUILD PRERELEASE)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
    set(SGX_MODE HW)
    set(SGX_BUILD RELEASE)
ELSE()
    message(FATAL_ERROR "unknown build type ${CMAKE_BUILD_TYPE}")
ENDIF()

include(cmake/ConfigSGX.cmake)

set(TC_ROOT_DIR ${CMAKE_SOURCE_DIR}/src)
set(MBEDTLS_LIB_DIR ${TC_ROOT_DIR}/Enclave/mbedtls-SGX)

# check cmake configuration
message(STATUS "Building ${CMAKE_BUILD_TYPE} with CFLAGS ${CMAKE_C_FLAGS}")

add_subdirectory(src/App)
add_subdirectory(src/Enclave)

if(BUILD_TESTS)
    add_subdirectory(src/vendor/gtest)
endif(BUILD_TESTS)