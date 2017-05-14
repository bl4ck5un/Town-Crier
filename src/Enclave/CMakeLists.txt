set(LIB_MBEDTLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX/lib/libmbedtls_sgx_t.a")

set(EXTERNAL_SRC_FILES
        external/keccak.c
        external/csv_parser.cpp
        external/slre.c
        external/gmtime.cpp
        external/http_parser.c)

set(TC_SOURCE_FILES
        env.cpp env.h
        init.cpp
        Debug.c Enclave_t.c enclave_exception_hdlr.c
        create_report.cpp event_handler.cpp
        commons.cpp encoding.cpp tls_client_tiny.cpp hybrid_cipher.cpp
        eth_ecdsa.cpp eth_transaction.cpp eth_abi.cpp
        ${LIB_MBEDTLS_PATH}
        ../Common/external/base64.cpp)

add_definitions(-DENCLAVE_STD_ALT)

FILE(GLOB TEST_SRC_FILES test/*.cpp)
FILE(GLOB SCRAPERS_SRC_FILES scrapers/*.cpp)

SET(SOURCE_FILES ${TC_SOURCE_FILES} ${TEST_SRC_FILES} ${SCRAPERS_SRC_FILES} ${EXTERNAL_SRC_FILES})

set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "Enclave_t.c; Enclave_t.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_library(enclave SHARED ${SOURCE_FILES})

include_directories(
        $(CMAKE_CURRENT_SOURCE_DIR)
        ${TC_ROOT_DIR}/Common
        mbedtls-SGX/include
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport)

set(CMAKE_C_FLAGS "${SGX_COMMON_CFLAGS} ${CMAKE_C_FLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

add_custom_command(
        OUTPUT ${LIB_MBEDTLS_PATH}
        COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX
)

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive \
    -Wl,--start-group ${LIB_MBEDTLS_PATH} -lsgx_tstdc -lsgx_tstdcxx -lsgx_tcrypto -l${SGX_TSVC_LIB} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

set(DEBUG_ENCLAVE_NAME "enclave.debug.so")
set(RELEASE_ENCLAVE_NAME "enclave.release.so")

add_custom_command(TARGET enclave
        POST_BUILD
        COMMAND ${SGX_ENCLAVE_SIGNER} sign
        -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -enclave enclave.so
        -out ${DEBUG_ENCLAVE_NAME}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMENT "Signing the enclave. Written to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_custom_command(TARGET enclave
        POST_BUILD
        COMMAND ${SGX_ENCLAVE_SIGNER} gendata
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -enclave enclave.so
        -out enclave.hex
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMENT "Generating signing materials. Written to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/enclave.hex")

add_custom_target(release-enclave
        DEPENDS enclave Enclave.config.xml release_pubkey.pem
        COMMAND ${SGX_ENCLAVE_SIGNER} catsig -enclave enclave.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -key ${CMAKE_CURRENT_SOURCE_DIR}/release_pubkey.pem
        -out ${RELEASE_ENCLAVE_NAME}
        -sig signature.hex
        -unsigned enclave.hex
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DEBUG_ENCLAVE_NAME} DESTINATION tc)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${RELEASE_ENCLAVE_NAME} DESTINATION tc)
