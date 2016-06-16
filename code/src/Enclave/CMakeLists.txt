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
        Transaction.cpp
        test_ssl.cpp)

set(CMAKE_VERBOSE_MAKEFILE False)

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
        mbedtls-SGX/include
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENCLAVE_STD_ALT ${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set(LIB_MBEDTLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX/lib/libmbedtls_sgx.a")

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${TRTS_LIB_NAME} -Wl,--no-whole-archive \
    -Wl,--start-group ${LIB_MBEDTLS_PATH} -lsgx_tstdc -lsgx_tstdcxx -l${CRYPTO_LIB_NAME} -l${SERVICE_LIB_NAME} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

add_custom_command(TARGET enclave
        POST_BUILD
        COMMAND ${SGX_ENCLAVE_SIGNER} sign
            -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
            -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
            -enclave enclave.so
            -out ${CMAKE_BINARY_DIR}/enclave.signed.so
        COMMENT "Signing the enclave. Written to ${CMAKE_BINARY_DIR}")

install(TARGETS enclave DESTINATION ${CMAKE_BINARY_DIR})