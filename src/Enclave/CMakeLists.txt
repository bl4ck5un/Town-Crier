set(SOURCE_FILES
    Debug.c
    create_report.cpp
    scrapers/current_bloomberg.cpp
    scrapers/current_Google.cpp
    scrapers/current_Yahoo.cpp
    scrapers/UPS_Tracking.cpp
    scrapers/current_coinmarket.cpp
    scrapers/current_weather.cpp
    scrapers/steam2.cpp
    eth_ecdsa.cpp
    Enclave_t.c
    event_handler.cpp
    scrapers/flight.cpp
    external/keccak.c
    tls_client.cpp
    scrapers/steam2.cpp
    scrapers/stock_ticker.cpp
    eth_transaction.cpp
    enclave_exception_hdlr.c
    eth_abi.cpp
    encoding.cpp
    external/slre.c
    external/http_parser.c utils.c)

add_definitions(-DENCLAVE_STD_ALT)

FILE(GLOB TEST_SRC_FILES test/*.cpp)

SET(SOURCE_FILES ${SOURCE_FILES} ${TEST_SRC_FILES}  scrapers/utils.h scrapers/UPS_Tracking.h scrapers/Scraper.cpp scrapers/Scraper.h scrapers/steam2.h scrapers/flight.h scrapers/stockticker.h)

set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "Enclave_t.c; Enclave_t.h")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
    COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
    --search-path ${TC_ROOT_DIR}/Enclave
    --search-path ${SGX_SDK}/include
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

include_directories(
    $(CMAKE_CURRENT_SOURCE_DIR)
    ${TC_ROOT_DIR}/Common
    mbedtls-SGX/include
    ${SGX_SDK}/include
    ${SGX_SDK}/include/tlibc
    ${SGX_SDK}/include/stlport
    .
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set(LIB_MBEDTLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX/lib/libmbedtls_sgx.a")

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive \
    -Wl,--start-group ${LIB_MBEDTLS_PATH} -lsgx_tstdc -lsgx_tstdcxx -lsgx_tcrypto -l${SGX_TSVC_LIB} -Wl,--end-group \
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
