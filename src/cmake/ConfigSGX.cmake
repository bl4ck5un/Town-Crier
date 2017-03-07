# get arch information
execute_process(COMMAND getconf LONG_BIT OUTPUT_VARIABLE VAR_LONG_BIT)
if (VAR_LONG_BIT STREQUAL 32)
    set(SGX_ARCH x86)
elseif(CMAKE_CXX_FLAGS MATCHES -m32)
    set(SGX_ARCH x86)
endif()

if (SGX_ARCH STREQUAL x86)
    message(FATAL_ERROR "only x64 build is supported.")
else()
    set(SGX_COMMON_CFLAGS -m64)
    set(SGX_LIBRARY_PATH ${SGX_SDK}/lib64)
    set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x64/sgx_sign)
    set(SGX_EDGER8R ${SGX_SDK}/bin/x64/sgx_edger8r)
endif()

# set debug flags properly
if (SGX_DEBUG AND SGX_PRERELEASE)
    message(FATAL_ERROR "Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time")
endif()

# figure out build mode
if (SGX_DEBUG)
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O0 -g")
else(SGX_DEBUG)
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O2")
endif(SGX_DEBUG)

if (SGX_DEBUG)
    # for debug mode: turn on DEBUG, turn off EDEBUG
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -UNDEBUG -UEDEBUG")
elseif (SGX_PRERELEASE)
    # for pre-release mode: turn off DEBUG, turn on EDEBUG
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -DEDEBUG")
else (SGX_DEBUG)
    # for release mode: turn off DEBUG, turn on EDEBUG
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -UEDEBUG")
endif (SGX_DEBUG)

if (SGX_MODE STREQUAL HW)
    set(SGX_URTS_LIB sgx_urts)
    set(SGX_USVC_LIB sgx_uae_service)
    set(SGX_TRTS_LIB sgx_trts)
    set(SGX_TSVC_LIB sgx_tservice)
else ()
    set(SGX_URTS_LIB sgx_urts_sim)
    set(SGX_USVC_LIB sgx_uae_service_sim)
    set(SGX_TRTS_LIB sgx_trts_sim)
    set(SGX_TSVC_LIB sgx_tservice_sim)
endif (SGX_MODE STREQUAL HW)

# set SGX_RELEASE if built in HW mode without DEBUG
# unless SGX_PRERELEASE is set
if (SGX_MODE STREQUAL HW AND NOT SGX_DEBUG AND NOT SGX_PRERELEASE)
    set(SGX_RELEASE TRUE)
else()
    set(SGX_RELEASE FALSE)
endif()

find_package(SGXSDK REQUIRED)

message(STATUS "SGX_COMMON_CFLAGS: ${SGX_COMMON_CFLAGS}")
message(STATUS "SGX_SDK: ${SGX_SDK}")
message(STATUS "SGX_ARCH: ${SGX_ARCH}")
message(STATUS "SGX_MODE: ${SGX_MODE}")
message(STATUS "SGX_PRERELEASE: ${SGX_PRERELEASE}")
message(STATUS "SGX_RELEASE: ${SGX_RELEASE}")
message(STATUS "SGX_LIBRARY_PATH: ${SGX_LIBRARY_PATH}")
message(STATUS "SGX_ENCLAVE_SIGNER: ${SGX_ENCLAVE_SIGNER}")
message(STATUS "SGX_EDGER8R: ${SGX_EDGER8R}")
