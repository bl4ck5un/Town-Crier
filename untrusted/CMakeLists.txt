set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

add_library(${PROJECT_NAME}_u STATIC mbedtls_sgx_u_net.c mbedtls_sgx_u_std.c)

include_directories(
        ../trusted/mbedtls-2.2.1/include
)