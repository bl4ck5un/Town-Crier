set(SOURCE_FILES
        App.cpp
        Converter.cpp
        Enclave_u.c
        EthRPC.cpp
        ethrpcclient.h
        Monitor.cpp
        ocalls.cpp
        printf.c
        RemoteAtt.cpp
        snprintf.c
        utils.cpp
        ../Common/Debug.c
        ../Common/Log.c
        mbedtls_sgx_net.c
        bookkeeping/database.cpp
        bookkeeping/transaction-record.hxx
        bookkeeping/transaction-record-odb.cxx
        StatusRpcServer.cpp StatusRpcServer.h types.cpp types.hxx request-parser.cpp request-parser.hxx)


set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(tc ${SOURCE_FILES})
add_executable(keygen keygen.cpp ../Common/Log.c utils.cpp Converter.cpp Enclave_u.c ocalls.cpp mbedtls_sgx_net.c)

# Tests
FILE(GLOB TEST_SRC_FILES "test/*.cpp")
add_executable(TestMain EXCLUDE_FROM_ALL
        ${TEST_SRC_FILES}
        utils.cpp
        Enclave_u.c
        ocalls.cpp
        EthRPC.cpp
        bookkeeping/database.cpp
        bookkeeping/transaction-record.hxx
        bookkeeping/transaction-record-odb.cxx
        mbedtls_sgx_net.c
        Converter.cpp
        ../Common/Debug.c
        ../Common/Log.c types.cpp types.hxx request-parser.cpp request-parser.hxx)

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else ()
    set(Urts_Library_Name sgx_urts)
endif ()

include_directories(
        /usr/local/include
        ../Common
        ${SGX_SDK}/include
        ${MBEDTLS_LIB_DIR}/include
        /usr/include/jsoncpp
        ../vendor/gtest/include
)

if (SGX_DEBUG STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -UNDEBUG -UEDEBUG")
elseif (SGX_PRERELEASE STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -DEDEBUG")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -UEDEBUG")
endif ()

if (SGX_MODE STREQUAL HW)
    set(UAE_SERVICE_LIB sgx_uae_service)
else ()
    set(UAE_SERVICE_LIB sgx_uae_service_sim)
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

target_link_libraries(tc
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB}
        -lsqlite3
        -ljsoncpp
        -lcurl
        -ljsonrpccpp-common
        -ljsonrpccpp-client
        -ljsonrpccpp-server
        -lmicrohttpd
        -lpthread
        -lodb-sqlite
        -lodb
        -lboost_system
        -lboost_filesystem
        )

target_link_libraries(keygen
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB} -lpthread
        )

set(LIBGTEST "${CMAKE_SOURCE_DIR}/vendor/gtest/lib/.libs")
set(TEST_PROGRAM_LDFLAGS " ${LIBGTEST}/libgtest_main.a")

target_link_libraries(TestMain
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB}
        -lsqlite3
        -ljsoncpp
        -lcurl
        -ljsonrpccpp-common
        -ljsonrpccpp-client
        -lpthread
        -lodb-sqlite
        -lodb
        ${LIBGTEST}/libgtest.a
        ${LIBGTEST}/libgtest_main.a
        )

add_custom_command(TARGET tc
        POST_BUILD
        COMMAND cp
        -f ${CMAKE_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}
        COMMENT "Testing command")

add_dependencies(tc enclave)
add_dependencies(TestMain enclave)
