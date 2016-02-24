#pragma once

// switch on banchmark

// at most one of them should be enabled.
//#define E2E_BENCHMARK
//#define E2E_BENCHMARK_THREADING
//#define OFFLINE_BENCHMARK
#define TIME_CALIBRATION_BENCHMARK

#if (defined(TIME_CALIBRATION_BENCHMARK) || defined(OFFLINE_BENCHMARK) || \
    defined(E2E_BENCHMARK_THREADING) || defined(E2E_BENCHMARK))
#define BENCHMARK
#endif

#define TX_BUF_SIZE 2048

#define TC_ADDRESS "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b"
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000

#define TYPE_CURRENT_VOTE   0x80
#define TYPE_FLIGHT_INS     0x81
#define TYPE_STEAM_EX       0x82
#define TYPE_FINANCE_INFO   0x83

#define FREQ 2500000.0