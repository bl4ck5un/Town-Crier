set(SOURCE_FILES
    ../Common/Debug.c
    ../Common/Log.c
    ABI.cpp
    Attestation_Enc.c
    Current_bloomberg.cpp
    Current_Google.cpp
    Current_Yahoo.cpp
    dispatcher.cpp
    ECDAS.c
    Enclave_t.c
    EventHandler.cpp
    Flight.cpp
    keccak.c
    RootCerts.c
    Scraper_lib.c
    SSLClient.c
    Steam2.cpp
    StockTicker.cpp
    Transaction.cpp)

set(CMAKE_VERBOSE_MAKEFILE FALSE)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(TRTS_LIB_NAME sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
else()
    set(TRTS_LIB_NAME sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
endif()

set(CRYPTO_LIB_NAME sgx_tcrypto)

include_directories(
    ${TC_ROOT_DIR}/Common
    ${TC_ROOT_DIR}/mbedtls-SGX/lib/include
    ${SGX_SDK}/include
    ${SGX_SDK}/include/tlibc
    ${SGX_SDK}/include/stlport
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENCLAVE_STD_ALT ${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set(LIB_MBEDTLS_PATH "${TC_ROOT_DIR}/mbedtls-SGX/lib/libmbedtls_sgx.a")

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${TRTS_LIB_NAME} -Wl,--no-whole-archive \
    -Wl,--start-group ${LIB_MBEDTLS_PATH} -lsgx_tstdc -lsgx_tstdcxx -l${CRYPTO_LIB_NAME} -l${SERVICE_LIB_NAME} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

add_custom_target(signed.enclave
        ${SGX_ENCLAVE_SIGNER} sign -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -enclave enclave.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml -out enclave.signed.so)

add_dependencies(signed.enclave enclave)

add_custom_command(TARGET signed.enclave
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TC_ROOT_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy enclave.signed.so ${TC_ROOT_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy enclave.so ${TC_ROOT_DIR}/bin
        COMMENT "Installing signed enclave files")