set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

set(TC_SRC_FILES monitor.cpp attestation.cpp request-parser.cpp
        mbedtls_sgx_net.c Enclave_u.c ocalls.cpp
        debug.cpp
        bookkeeping/database.cpp
        bookkeeping/transaction-record.hxx
        bookkeeping/transaction-record-odb.cxx
        Converter.cpp utils.cpp types.cpp
        EthRPC.cpp StatRPCServer.cpp
        ../Common/external/base64.cpp
        key-utils.cpp
        tc-exception.cpp config.cpp config.h)

set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "Enclave_u.c; Enclave_u.h")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${TC_ROOT_DIR}/Enclave/Enclave.edl
        --search-path ${TC_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(tc tc.cpp ${TC_SRC_FILES})
add_executable(tc-oneshot tc-oneshot.cpp ${TC_SRC_FILES})
add_executable(tc-keygen
        tc-keygen.cpp
        ../Common/external/base64.cpp
        utils.cpp Converter.cpp
        Enclave_u.c ocalls.cpp mbedtls_sgx_net.c)

# Source files for tests
FILE(GLOB TEST_SRC_FILES "test/*.cpp")
add_executable(TestMain EXCLUDE_FROM_ALL
        ${TEST_SRC_FILES}
        test/SGXTestBase.h
        attestation.cpp
        utils.cpp
        debug.cpp
        Enclave_u.c
        ocalls.cpp
        EthRPC.cpp
        bookkeeping/database.cpp
        bookkeeping/transaction-record.hxx
        bookkeeping/transaction-record-odb.cxx
        mbedtls_sgx_net.c
        Converter.cpp
        ../Common/external/base64.cpp
        types.cpp request-parser.cpp
        key-utils.cpp test/test-data.h)

find_package(CURL REQUIRED)
find_package(Boost QUIET REQUIRED COMPONENTS program_options filesystem system)
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
        ../
)

target_link_libraries(tc
        ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
        ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
        ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY}
        -lpthread -ldl)

target_link_libraries(tc-oneshot
        ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
        ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
        ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY}
        -lpthread -ldl)

target_link_libraries(tc-keygen
        ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY} ${Boost_LIBRARIES}
        -lpthread -ldl)

set(LIBGTEST "${CMAKE_SOURCE_DIR}/vendor/gtest/lib/.libs")
set(TEST_PROGRAM_LDFLAGS " ${LIBGTEST}/libgtest_main.a")

target_link_libraries(TestMain
        ${SGX_URTS_LIBRARY} ${SGX_USVC_LIBRARY} ${Boost_LIBRARIES}
        ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES} ${JSONRPCCPP_LIBRARIES}
        ${LIBODB_LIBRARIES} ${MHD_LIBRARIES} ${SQLITE_LIBRARIES} ${Boost_LIBRARIES}
        -lpthread -ldl
        ${LIBGTEST}/libgtest.a
        ${LIBGTEST}/libgtest_main.a)

add_custom_command(TARGET tc
        POST_BUILD
        COMMAND cp -f ${CMAKE_SOURCE_DIR}/config-* ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Installing the configuration files")


add_dependencies(tc enclave)
add_dependencies(TestMain enclave)
add_dependencies(tc-keygen enclave)

install(TARGETS tc DESTINATION tc)
install(FILES ${CMAKE_SOURCE_DIR}/config-systemd DESTINATION tc)
install(CODE "message(
\"Please run
    sudo systemctl enable tc
or
    sudo systemctl restart tc
to (re)load tc.service\")")