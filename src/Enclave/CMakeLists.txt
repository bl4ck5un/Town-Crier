add_definitions(-DENCLAVE_STD_ALT)
add_definitions(-DIN_ENCLAVE)

###############################
#### list source code files
###############################

set(LIB_MBEDTLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX/lib/libmbedtls_sgx_t.a")

set(EXTERNAL_SRC_FILES
        external/keccak.c
        external/csv_parser.cpp
        external/slre.c
        external/gmtime.cpp
        external/http_parser.c)

set(TC_SOURCE_FILES
        commons.cpp
        debug.c
        env.cpp
        log.cpp
        Enclave_t.c
        enclave_exception_hdlr.c
        ecall_attestation.cpp
        event_handler.cpp
        encoding.cpp
        tls_client_tiny.cpp
        hybrid_cipher.cpp
        eth_ecdsa.cpp
        eth_transaction.cpp
        pedersen_commit.cpp
        levenshtein.cpp
        eth_abi.cpp
        ../Common/external/base64.cpp)

FILE(GLOB TEST_SRC_FILES test/*.cpp)
FILE(GLOB SCRAPERS_SRC_FILES scrapers/*.cpp)


set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "Enclave_t.c; Enclave_t.h")

SET(SOURCE_FILES ${TC_SOURCE_FILES} ${TEST_SRC_FILES} ${SCRAPERS_SRC_FILES} ${EXTERNAL_SRC_FILES})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

###############################
#### targets
###############################

add_library(enclave SHARED ${SOURCE_FILES})

###############################
#### Include & Libs
###############################

add_custom_command(TARGET enclave
        PRE_BUILD
        COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-SGX
        )

set(CMAKE_C_FLAGS "${SGX_COMMON_CFLAGS} ${CMAKE_C_FLAGS} \
    -Wall -Werror=return-type -nostdinc -fvisibility=hidden -fpie -fstack-protector")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} \
    -std=c++11 -nostdinc++")

include_directories(
        $(CMAKE_CURRENT_SOURCE_DIR)
        ${TC_ROOT_DIR}/Common
        mbedtls-SGX/include
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/libcxx)

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive \
    -Wl,--start-group ${LIB_MBEDTLS_PATH} -lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -l${SGX_TSVC_LIB} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

###############################
#### Output & Installtation
###############################

set(DEBUG_ENCLAVE_NAME "enclave.debug.so")

add_custom_command(TARGET enclave
        POST_BUILD
        COMMAND ${SGX_ENCLAVE_SIGNER} sign
        -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -enclave libenclave.so
        -out ${DEBUG_ENCLAVE_NAME}
        COMMENT "Signing the ${CMAKE_CURRENT_BINARY_DIR}/libenclave.so.")

add_custom_command(TARGET enclave
        POST_BUILD
        COMMAND ${SGX_ENCLAVE_SIGNER} gendata
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -enclave libenclave.so
        -out enclave.hex
        COMMENT "Generating signing materials. Written to ${CMAKE_CURRENT_BINARY_DIR}/enclave.hex")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEBUG_ENCLAVE_NAME} DESTINATION enclave)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/enclave.hex
        ${CMAKE_CURRENT_SOURCE_DIR}/release_pubkey.pem
        ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        ${CMAKE_CURRENT_BINARY_DIR}/libenclave.so
        DESTINATION signing
        CONFIGURATIONS Release)
