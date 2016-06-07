cmake_minimum_required(VERSION 3.2)
project(TC_Linux)

set(SGX_SDK /opt/intel/sgxsdk)
set(SGX_MODE HW)
set(SGX_ARCH x64)
set(SGX_DEBUG 1)

set(TC_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(MBEDTLS_LIB_DIR ${TC_ROOT_DIR}/mbedtls-SGX/lib)

set(CMAKE_VERBOSE_MAKEFILE False)

execute_process(COMMAND getconf LONG_BIT OUTPUT_VARIABLE VAR_LONG_BIT)
if ("${LONG_BIT}" STREQUAL "32")
    set(SGX_ARCH x86)
elseif(${CMAKE_CXX_FLAGS} MATCHES "-m32")
    set(SGX_ARCH x86)
endif()


if (${SGX_ARCH} STREQUAL "x86")
    set(SGX_COMMON_CFLAGS -m32)
    set(SGX_LIBRARY_PATH ${SGX_SDK}/lib)
    set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x86/sgx_sign)
    set(SGX_EDGER8R ${SGX_SDK}/bin/x86/sgx_edger8r)
else()
    set(SGX_COMMON_CFLAGS -m64)
    set(SGX_LIBRARY_PATH ${SGX_SDK}/lib64)
    set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x64/sgx_sign)
    set(SGX_EDGER8R ${SGX_SDK}/bin/x64/sgx_edger8r)
endif()

if (SGX_DEBUG STREQUAL 1 AND SGX_PRERELEASE STREQUAL 1)
    message(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif()

if (${SGX_DEBUG} EQUAL 1)
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O0 -g")
else()
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O2")
endif()

message("status SGX_COMMON_CFLAGS: ${SGX_COMMON_CFLAGS}")
message("status SGX_SDK: ${SGX_SDK}")
message("status SGX_ARCH: ${SGX_ARCH}")
message("status SGX_MODE: ${SGX_MODE}")
message("status SGX_LIBRARY_PATH: ${SGX_LIBRARY_PATH}")
message("status SGX_ENCLAVE_SIGNER: ${SGX_ENCLAVE_SIGNER}")
message("status SGX_EDGER8R: ${SGX_EDGER8R}")

add_subdirectory(App)