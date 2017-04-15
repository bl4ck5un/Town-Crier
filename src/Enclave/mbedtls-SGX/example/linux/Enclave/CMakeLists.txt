set(SOURCE_FILES
        ecalls.cpp
        Enclave_t.c
        Log.c
        pprint.c
        s_client.c
        s_server.c
        ssl_conn_hdlr.cpp
        )

add_definitions(-DENCLAVE_STD_ALT)

# TODO: correct the typo in Town Cirer (_u -> _t)
set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "Enclave_t.c; Enclave_t.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
        --search-path ${SGX_SDK}/include
        --search-path ${MBEDTLS_SGX_ROOT_DIR}/lib
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

include_directories(
        $(CMAKE_CURRENT_SOURCE_DIR)
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport
        ${MBEDTLS_SGX_ROOT_DIR}/include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set(LIB_MBEDTLS_PATH "${MBEDTLS_LIB_DIR}/libmbedtls_sgx_t.a")

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