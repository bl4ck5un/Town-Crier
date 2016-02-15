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

static int stock_ticker_handler(uint8_t* nonce, uint64_t request_id, uint8_t request_type, 
                   const uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
    long long time1, time2;
    rdtsc(&time1);
    rdtsc(&time2);
    LL_CRITICAL("ctx swtich down: %llu", time1);

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

    rdtsc(&time1);
    int price = (int) get_closing_price(12, 3, 2014, "BABA");
    rdtsc(&time2);
    LL_CRITICAL("SSL fetch %llu", time2 - time1);

    LL_NOTICE("Closing pricing is %d", price);

    bytes rr;
    enc_int(rr, price, sizeof price);;

    return get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        &rr[0], 32, 
        raw_tx, raw_tx_len);
}

static int flight_insurance_handler()
{
    long long time1, time2;
    int ret, delay;

    rdtsc(&time1);
    LL_CRITICAL("ctx swtich done:  %llu", time1);
    ret = get_flight_delay("20160215", "0655", "UAL1183", &delay);
    rdtsc(&time2);
    LL_CRITICAL("get_flight_delay: %llu", time2-time1);
    LL_CRITICAL("delay is %d", delay);

    bytes rr;
    enc_int(rr, delay, sizeof delay);;

    uint8_t req[64];
    uint8_t raw_tx[1024];
    int raw_tx_len = sizeof raw_tx;
    uint8_t nonce[32] = {9};

    return get_raw_signed_tx(nonce, 32, 
        1, 1, 
        req, sizeof req, 
        &rr[0], 32, 
        raw_tx, &raw_tx_len);
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
    case REQ_TYPE_FINANCE:
        return flight_insurance_handler();
        return stock_ticker_handler(nonce, request_id, request_type, 
            req, req_len, 
            raw_tx, raw_tx_len);
        break;
    default :
        LL_CRITICAL("Unknown request type: %d", request_type);
        return -1;
        break;
    }
}

