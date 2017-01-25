#pragma once

// switch on banchmark

// at most one of them should be enabled.
//#define E2E_BENCHMARK
//#define E2E_BENCHMARK_THREADING

//#define OFFLINE_BENCHMARK
//#define TIME_CALIBRATION_BENCHMARK

#if (defined(TIME_CALIBRATION_BENCHMARK) || defined(OFFLINE_BENCHMARK) || \
    defined(E2E_BENCHMARK_THREADING) || defined(E2E_BENCHMARK))
#define BENCHMARK
#endif

#define TX_BUF_SIZE 2048

#define TC_ADDRESS  "0x78aafb07c911d44009f4ecd60f31c84b7735322d"
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000

#define TYPE_CURRENT_VOTE   0x0
#define TYPE_FLIGHT_INS     0x1
#define TYPE_STEAM_EX       0x2
#define TYPE_FINANCE_INFO   0x3

#define SGX_ADDRESS "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"

#define ERR_ENCLAVE_SSL_CLIENT 0x1001 

//Define flight status constants 
#define INVALID -1
#define DEPARTURED 0
#define DELAYED 1
#define CANCELLED 2
#define NOT_DEPARTURED 3
#define NOT_FOUND 4

//Define UPS Tracking Constant
#define PACKAGE_NOT_FOUND 0
#define ORDER_PROCESSED 1
#define SHIPPED 2
#define IN_TRANSIT 3
#define OUT_FOR_DELIVERY 4
#define DELIVERED 5

typedef int pkgstatus_t;

