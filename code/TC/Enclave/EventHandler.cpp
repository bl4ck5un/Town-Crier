#include "stdint.h"
#include "EventHandler.h"
#include <string>
#include <Log.h>
#include "dispatcher.h"
#include "time.h"
#include "Transaction.h"
#include "ABI.h"
#include "Enclave_t.h"
#include "keccak.h"
#include <Debug.h>
#include "Constants.h"

static int stock_ticker_handler(uint8_t* nonce, uint64_t request_id, uint8_t request_type, 
                   const uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
    int ret;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done: %llu", time1);
#endif
    if (req_len != 64)
    {
        LL_CRITICAL("req_len is not 64");
        return -1;
    }
    
    char* code = (char*)(req);
    uint32_t date;
    time_t epoch;
    memcpy(&date, req + 64 - sizeof uint32_t, sizeof uint32_t);
    date = swap_uint32(date);

    epoch = date;
    LL_NOTICE("Looking for %s at %lld", code, epoch);
#ifdef E2E_BENCHMARK
    rdtsc(&time1);
#endif
    int price = (int) get_closing_price(12, 3, 2014, "BABA");
#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_closing_price: %llu", time2 - time1);
#endif
    LL_NOTICE("Closing pricing is %d", price);

    bytes rr;
    enc_int(rr, price, sizeof price);;

    ret = get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        &rr[0], 32, 
        raw_tx, raw_tx_len);
#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

static int flight_insurance_handler()
{

    int ret, delay;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done:  %llu", time1);
#endif
    ret = get_flight_delay("20160215", "0655", "UAL1183", &delay);
#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_flight_delay: %llu", time2-time1);
#endif
    LL_NOTICE("delay is %d", delay);

    bytes rr;
    enc_int(rr, delay, sizeof delay);;

    uint8_t req[64];
    uint8_t raw_tx[1024];
    int raw_tx_len = sizeof raw_tx;
    uint8_t nonce[32] = {9};

    ret = get_raw_signed_tx(nonce, 32, 
        1, 1, 
        req, sizeof req, 
        &rr[0], 32, 
        raw_tx, &raw_tx_len);

#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

static int steam_exchange()
{

    int ret, rc;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done:  %llu", time1);
#endif
    char * listB[1] = {"Portal"};
    rc = get_steam_transaction(listB, 1, "32884794", 1355220300, "7978F8EDEF9695B57E72EC468E5781AD", &ret);
    if (rc == 0 && ret == 1) {
        LL_NOTICE("Found a trade, %d, %d", rc, ret);
    }
#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_flight_delay: %llu", time2-time1);
#endif

    bytes rr;
    enc_int(rr, rc, sizeof rc);;

    uint8_t req[64];
    uint8_t raw_tx[1024];
    int raw_tx_len = sizeof raw_tx;
    uint8_t nonce[32] = {9};

    ret = get_raw_signed_tx(nonce, 32, 
        1, 1, 
        req, sizeof req, 
        &rr[0], 32, 
        raw_tx, &raw_tx_len);

#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

//request(uint8 type, address cb, bytes4 cb_fid, bytes32[] req)
int handle_request(uint8_t* nonce, uint64_t request_id, uint8_t request_type, 
                   uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
//    uint8_t md[32] = {0};
//    uint8_t in[1 + 32 * 3] = {0};
//    in[0] = 123;
//    in[31 + 1] = 11;
//    in[63 + 1] = 22;
//    in[95 + 1] = 33;
//    keccak(in, sizeof in, md, sizeof md);
//    hexdump("Hash Input", in, sizeof in);
//    hexdump("Hash Test", md, 32);
//    return -1;

    switch (request_type)
    {
    case TYPE_FINANCE_INFO:
        return stock_ticker_handler(nonce, request_id, request_type, 
            req, req_len, 
            raw_tx, raw_tx_len);
        break;
    case TYPE_FLIGHT_INS:
        return flight_insurance_handler();
    case TYPE_STEAM_EX:
        return steam_exchange();
    case TYPE_CURRENT_VOTE:
        {
        double r1 = 0, r2 = 0, r3 = 0;
        long long time1, time2;

        rdtsc(&time1);
        yahoo_current("GOOG", &r1);
        rdtsc(&time2);
        LL_CRITICAL("Yahoo: %llu", time2-time1);

        google_current("GOOG", &r3);
        rdtsc(&time1);
        LL_CRITICAL("Bloomberg: %llu", time1-time2);

        google_current("GOOG", &r2);
        rdtsc(&time2);
        LL_CRITICAL("GOOGLE: %llu", time2-time1);

        return 0;
        break;
        }
    default :
        LL_CRITICAL("Unknown request type: %d", request_type);
        return -1;
        break;
    }
}

