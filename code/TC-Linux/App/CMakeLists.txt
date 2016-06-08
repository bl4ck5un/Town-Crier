set(SOURCE_FILES
        App.cpp
        Bookkeeping.cpp
        Enclave_u.c
        EthRPC.cpp
        ethrpcclient.h
        Monitor.cpp
        ocalls.cpp
        printf.c
        RemoteAtt.cpp
        snprintf.c
        Utils.cpp
        ../Common/Debug.c
        ../Common/Log.c
        )

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

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else()
    set(Urts_Library_Name sgx_urts)
endif()

include_directories(
    ../Common
    ${SGX_SDK}/include
    ${MBEDTLS_LIB_DIR}/include
    /usr/include/jsoncpp)


if(SGX_DEBUG STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -UNDEBUG -UEDEBUG")
elseif(SGX_PRERELEASE STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -DEDEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -UEDEBUG")
endif()

if(${SGX_MODE} STREQUAL HW)
    set(UAE_SERVICE_LIB sgx_uae_service)
else()
    set(UAE_SERVICE_LIB sgx_uae_service_sim)
endif()

find_library(LIB_UAE_SERVICE_PATH ${UAE_SERVICE_LIB} PATHS ${SGX_LIBRARY_PATH} NO_DEFAULT_PATH)
find_library(LIB_URTS_PATH ${Urts_Library_Name} PATHS ${SGX_LIBRARY_PATH} NO_DEFAULT_PATH)
find_library(LIB_MBEDTLS_U_PATH mbedtls_sgx_u PATHS ${MBEDTLS_LIB_DIR} NO_DEFAULT_PATH)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

target_link_libraries(tc
#        ${LIB_URTS_PATH}
#        ${LIB_UAE_SERVICE_PATH}
#        ${LIB_MBEDTLS_U_PATH}
#        sqlite3 jsoncpp curl jsonrpccpp-common jsonrpccpp-client)
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB}
        -L${TC_ROOT_DIR}/mbedtls-SGX/lib -lmbedtls_sgx_u
        -lsqlite3
        -ljsoncpp
        -lcurl
        -ljsonrpccpp-common
        -ljsonrpccpp-client)


add_dependencies(tc signed.enclave)

add_custom_command(TARGET tc
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TC_ROOT_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy tc ${TC_ROOT_DIR}/bin/
        COMMENT "Installing Town Crier exectuable")