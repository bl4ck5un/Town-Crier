set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

###############################
#### list source code files
###############################

set(TC_SRC_FILES
    monitor.cpp
    attestation.cpp
    request_parser.cpp
    mbedtls_sgx_net.c
    Enclave_u.c
    ocalls.cpp
    debug.cpp
    bookkeeping/database.cpp
    bookkeeping/transaction-record.hxx
    bookkeeping/transaction-record-odb.cxx
    converter.cpp utils.cpp types.cpp
    eth_rpc.cpp
    status_rpc_server.cpp
    ../Common/external/base64.cpp
    key_utils.cpp
    logging.cpp
    tc_exception.cpp
    config.cpp config.h)

FILE(GLOB TEST_SRC_FILES "test/*.cpp")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
    COMMAND ${SGX_EDGER8R} --untrusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
    --search-path ${TC_ROOT_DIR}/Enclave
    --search-path ${SGX_SDK}/include
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# generate version header
configure_file(
    ${CMAKE_SOURCE_DIR}/Common/version.h.in
    ${CMAKE_SOURCE_DIR}/Common/version.h
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

###############################
#### targets
###############################

add_executable(tc tc.cpp ${TC_SRC_FILES})
add_executable(tc-oneshot tc-oneshot.cpp ${TC_SRC_FILES})
add_executable(tc-keygen
    tc-keygen.cpp
    logging.cpp
    ../Common/external/base64.cpp
    utils.cpp converter.cpp
    Enclave_u.c ocalls.cpp mbedtls_sgx_net.c)

add_executable(tc-test EXCLUDE_FROM_ALL
    ${TEST_SRC_FILES}
    test/SGXTestBase.h
    attestation.cpp
    utils.cpp
    debug.cpp
    Enclave_u.c
    ocalls.cpp
    eth_rpc.cpp
    logging.cpp
    bookkeeping/database.cpp
    bookkeeping/transaction-record.hxx
    bookkeeping/transaction-record-odb.cxx
    mbedtls_sgx_net.c
    converter.cpp
    ../Common/external/base64.cpp
    types.cpp request_parser.cpp
    key_utils.cpp test/test-data.h)

###############################
#### dependencies
###############################

find_package(CURL REQUIRED)
find_package(Boost QUIET REQUIRED COMPONENTS program_options filesystem system)
find_package(Sqlite REQUIRED)
find_package(LibODB REQUIRED)
find_package(Jsoncpp REQUIRED)
find_package(MHD REQUIRED)
find_package(JsonCppRpc REQUIRED)
find_package(Log4CXX REQUIRED)

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
    ${LOG4CXX_INCLUDE_DIR}
    ../vendor/gtest/include
    ../
)

target_link_libraries(tc
    ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
    ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
    ${LOG4CXX_LIBRARY}
    ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY}
    -lpthread -ldl)

target_link_libraries(tc-oneshot
    ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
    ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
    ${LOG4CXX_LIBRARY}
    ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY}
    -lpthread -ldl)

target_link_libraries(tc-keygen
    ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY} ${Boost_LIBRARIES}
    ${LOG4CXX_LIBRARY}
    -lpthread -ldl)

set(LIBGTEST "${CMAKE_SOURCE_DIR}/vendor/gtest/lib/.libs")
set(TEST_PROGRAM_LDFLAGS " ${LIBGTEST}/libgtest_main.a")

target_link_libraries(tc-test
    ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY} ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
    ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
    ${LOG4CXX_LIBRARY}
    -lpthread -ldl
    ${LIBGTEST}/libgtest.a
    ${LIBGTEST}/libgtest_main.a)

add_dependencies(tc enclave)
add_dependencies(tc-test enclave)
add_dependencies(tc-keygen enclave)

###############################
#### Output & Installtation
###############################

install(TARGETS tc tc-oneshot tc-keygen DESTINATION bin)
install(FILES
        ${CMAKE_SOURCE_DIR}/config-systemd
        ${CMAKE_SOURCE_DIR}/config-sim-test
        ${CMAKE_SOURCE_DIR}/config-release
        ${CMAKE_SOURCE_DIR}/logging.conf
        DESTINATION conf)
