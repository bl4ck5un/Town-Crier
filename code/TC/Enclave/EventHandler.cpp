#include "stdint.h"
#include "EventHandler.h"
#include <string>
#include <Log.h>
#include "dispatcher.h"

static int stock_ticker_handler(uint64_t request_id, uint8_t request_type, 
                   char* req, int req_len, char* resp, int resp_len);

//request(uint8 type, address cb, bytes4 cb_fid, bytes32[] req)
int handle_request(uint64_t request_id, uint8_t request_type, 
                   char* req, int req_len, char* resp, int resp_len)
{
    switch (request_type)
    {
    case REQ_TYPE_FINANCE:
        return stock_ticker_handler(request_id, request_type, req, req_len, resp, resp_len);
        break;
    default :
        return -1;
        break;
    }
}

static int stock_ticker_handler(uint64_t request_id, uint8_t request_type, 
                   char* req, int req_len, char* resp, int resp_len)
{
    if (req_len != 64)
    {
        LL_CRITICAL("req_len is not 64");
        return -1;
    }
    char code[32];
    char date[32];
    memcpy(code, req, 32);
    memcpy(date, req + 32, 32);
    int price = get_closing_price(12, 3, 2014, "BABA");
    // XXX STOP HERE// Fan
    return 0;
}