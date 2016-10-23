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

#define TC_ADDRESS "0xd8ecae22aa26953488e0c91acb6120db0be74cda"
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000

#define TYPE_CURRENT_VOTE   0x0
#define TYPE_FLIGHT_INS     0x1
#define TYPE_STEAM_EX       0x2
#define TYPE_FINANCE_INFO   0x3

#define FREQ 2500000.0

#define ERR_ENCLAVE_SSL_CLIENT 0x1001 

//Define flight status constants 
#define DEPARTURED 0
#define NOT_DEPARTURED 1
#define INVALID 2
