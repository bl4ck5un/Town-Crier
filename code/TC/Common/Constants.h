#pragma once

// switch on banchmark
//#define E2E_BENCHMARK
#define E2E_BENCHMARK_THREADING

#define TX_BUF_SIZE 2048

#define TC_ADDRESS "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b"
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000

#define TYPE_CURRENT_VOTE   0x80
#define TYPE_FLIGHT_INS     0x81
#define TYPE_STEAM_EX       0x82
#define TYPE_FINANCE_INFO   0x83