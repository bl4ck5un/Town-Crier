#pragma once

#define TX_BUF_SIZE 2048

#define TC_ADDRESS  "0x18322346bfb90378ceaf16c72cee4496723636b9"
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000

/* Define any events that Town Crier Supports here */
#define TYPE_CURRENT_VOTE   0x0
#define TYPE_FLIGHT_INS     0x1
#define TYPE_STEAM_EX       0x2
#define TYPE_FINANCE_INFO   0x3

/* Possible return values to be used in Enclave-Relay Connections */
#define TC_SUCCESS  		0x0
#define TC_INPUT_ERROR 		0x1
#define TC_INTERNAL_ERROR 	0x3

#define SGX_ADDRESS "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"

#define ERR_ENCLAVE_SSL_CLIENT 0x1001

/* flags for enclave debugging and logging */
// #define TRACE_TLS_CLIENT
// #define HEXDUMP_TLS_TRANSCRIPT

typedef int pkgstatus_t;