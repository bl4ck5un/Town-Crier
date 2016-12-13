set(CMAKE_VERBOSE_MAKEFILE False)

set(SOURCE_FILES
        App.cpp
        Bookkeeping.cpp
        ../Common/Converter.cpp
        Enclave_u.c
        EthRPC.cpp
        ethrpcclient.h
        Monitor.cpp
        ocalls.cpp
        printf.c
        RemoteAtt.cpp
        snprintf.c
        Init.cpp
        Utils.cpp
        ../Common/Debug.c
        ../Common/Log.c
        mbedtls_sgx_net.c
        )

FILE(GLOB TEST_SRC_FILES "test/*.cpp")

set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
        OUTPUT ethrpcclient.h
        COMMAND jsonrpcstub ${TC_ROOT_DIR}/App/eth_api.json --cpp-client ethRPCClient
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(tc ${SOURCE_FILES})

# Tests
add_executable(TestMain
        ${TEST_SRC_FILES}
        Init.cpp
        Enclave_u.c
        Utils.cpp
        ocalls.cpp
        EthRPC.cpp
        mbedtls_sgx_net.c
        ../Common/Converter.cpp
        ../Common/Debug.c
        ../Common/Log.c test/Sealing.cpp)

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else ()
    set(Urts_Library_Name sgx_urts)
endif ()

include_directories(
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
        -ljsonrpccpp-client)

set(LIBGTEST "${CMAKE_SOURCE_DIR}/vendor/gtest/lib/.libs")
set(TEST_PROGRAM_LDFLAGS " ${LIBGTEST}/libgtest_main.a")

target_link_libraries(TestMain
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB}
        -lsqlite3
        -ljsoncpp
        -lcurl
        -ljsonrpccpp-common
        -ljsonrpccpp-client
        ${LIBGTEST}/libgtest.a
        ${LIBGTEST}/libgtest_main.a
        -lpthread)


add_dependencies(tc enclave)
add_dependencies(TestMain enclave)
