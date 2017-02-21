set(SOURCE_FILES
        ../Common/Debug.c
        ../Common/Log.c
        eth_abi.cpp
        create_report.cpp
        scrapers/current_bloomberg.cpp
        scrapers/current_Google.cpp
        scrapers/current_Yahoo.cpp
        scrapers/UPS_Tracking.cpp
        scrapers/current_coinmarket.cpp
        scrapers/current_weather.cpp
        eth_ecdsa.c
        Enclave_t.c
        event_handler.cpp
        scrapers/flight.cpp
        external/keccak.c
        trusted_ca_certs.c
        tls_client.cpp
        scrapers/steam2.cpp
        scrapers/stock_ticker.cpp
        eth_transaction.cpp
        enclave_exception_hdlr.c
        encoding.cpp
        external/slre.c
        external/http_parser.c)

FILE(GLOB TEST_SRC_FILES test/*.cpp)

SET(SOURCE_FILES ${SOURCE_FILES} ${TEST_SRC_FILES})

set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(TRTS_LIB_NAME sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
else ()
    set(TRTS_LIB_NAME sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
endif ()

set(CRYPTO_LIB_NAME sgx_tcrypto)

include_directories(
        $(CMAKE_CURRENT_SOURCE_DIR)
        ${TC_ROOT_DIR}/Common
        mbedtls-SGX/include
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport
        .
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
        -enclave ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/enclave.so
        -out ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/enclave.signed.so
        COMMENT "Signing the enclave. Written to ${CMAKE_BINARY_DIR}")
