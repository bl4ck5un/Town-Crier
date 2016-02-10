#include "stdint.h"
#include "EventHandler.h"
#include <string>
#include <Log.h>
#include "dispatcher.h"
#include "time.h"
#include "Transaction.h"
#include "ABI.h"

static int stock_ticker_handler(uint8_t*nonce, uint64_t request_id, uint8_t request_type, 
                   const uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
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
    int price = (int) get_closing_price(12, 3, 2014, "BABA");
    LL_NOTICE("Closing pricing is %d", price);

    bytes rr;
    enc_int(rr, price, sizeof price);;

    return get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        &rr[0], 32, 
        raw_tx, raw_tx_len);
}

//request(uint8 type, address cb, bytes4 cb_fid, bytes32[] req)
int handle_request(uint8_t* nonce, uint64_t request_id, uint8_t request_type, 
                   uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
    switch (request_type)
    {
    case REQ_TYPE_FINANCE:
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

