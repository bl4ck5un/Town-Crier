set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

set(TC_SRC_FILES tc.cpp monitor.cpp attestation.cpp request-parser.cpp
    mbedtls_sgx_net.c Enclave_u.c ocalls.cpp
    bookkeeping/database.cpp
    bookkeeping/transaction-record.hxx
    bookkeeping/transaction-record-odb.cxx
    Converter.cpp utils.cpp types.cpp
    EthRPC.cpp StatusRpcServer.cpp StatusRpcServer.h
    ../Common/Debug.c
        )

set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
    COMMAND ${SGX_EDGER8R} --untrusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
    --search-path ${TC_ROOT_DIR}/Enclave
    --search-path ${SGX_SDK}/include
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(tc ${TC_SRC_FILES})
add_executable(keygen
        tc-keygen.cpp
        external/base64.cpp
        utils.cpp Converter.cpp
        Enclave_u.c ocalls.cpp mbedtls_sgx_net.c)
add_executable(tc-att tc-att.cpp attestation.cpp Converter.cpp ../Common/Debug.c utils.cpp Enclave_u.c ocalls.cpp mbedtls_sgx_net.c)

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
        types.cpp types.hxx request-parser.cpp request-parser.hxx)

find_package(CURL REQUIRED)
find_package(Boost QUIET REQUIRED COMPONENTS program_options filesystem system log)
find_package(Sqlite REQUIRED)
find_package(LibODB REQUIRED)
find_package(Jsoncpp REQUIRED)
find_package(MHD REQUIRED)
find_package(JsonCppRpc REQUIRED)

include_directories(
    ${CURL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${LIBODB_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}/${JSONCPP_INCLUDE_PREFIX}
    ${MHD_INCLUDE_DIRS}
    ${SGXSDK_INCLUDE_DIRS}
    ${JSONRPCCPP_INCLUDE_DIRS}
    ${MBEDTLS_LIB_DIR}/include
    ../vendor/gtest/include
    ../Common
)

target_link_libraries(tc
    ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
    ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
    -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -l${SGX_USVC_LIB}
    -Wl,-rpath -Wl,/usr/local/lib
    -Wl,-rpath -Wl,${SGX_LIBRARY_PATH}
    -lpthread -ldl)

target_link_libraries(keygen
    -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -l${SGX_USVC_LIB} ${Boost_LIBRARIES}
    -lpthread -ldl)

target_link_libraries(tc-att
    -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -l${SGX_USVC_LIB} ${Boost_LIBRARIES}
    -lpthread -ldl)

set(LIBGTEST "${CMAKE_SOURCE_DIR}/vendor/gtest/lib/.libs")
set(TEST_PROGRAM_LDFLAGS " ${LIBGTEST}/libgtest_main.a")

target_link_libraries(TestMain
    -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -l${SGX_USVC_LIB}
    ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
    ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
    -lpthread
    ${LIBGTEST}/libgtest.a
    ${LIBGTEST}/libgtest_main.a
    -Wl,-rpath -Wl,/usr/local/lib
    -Wl,-rpath -Wl,${SGX_LIBRARY_PATH})

add_custom_command(TARGET tc
    POST_BUILD
    COMMAND cp
    -f ${CMAKE_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}
    COMMENT "Installing the default configuration file")

add_dependencies(tc enclave)
add_dependencies(TestMain enclave)
