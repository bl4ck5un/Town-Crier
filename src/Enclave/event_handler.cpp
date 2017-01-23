#include "stdint.h"
#include "event_handler.h"
#include <string>
#include <Log.h>
#include "scrapers.h"
#include "time.h"
#include "eth_transaction.h"
#include "eth_abi.h"
#include "Enclave_t.h"
#include "external/keccak.h"
#include <Debug.h>
#include "Constants.h"

static int stock_ticker_handler(int nonce, uint64_t request_id, uint8_t request_type, 
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
    memcpy(&date, req + 64 - sizeof (uint32_t), sizeof (uint32_t));
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
    enc_int(rr, price, sizeof (price));;

    ret = get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        rr,
        raw_tx, raw_tx_len);
#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

static int flight_insurance_handler(uint8_t *req, int len, int *resp_data)
{
/*
 *  0x00 - 0x20 string flight_number
 *  ox20 - 0x40 uint64 unix_epoch
 */

    dump_buf("Request", req, len);

    int ret, delay;
    char flight_number[35] = {0};
    memcpy(flight_number, req, 0x20);
    
    char* flighttime = (char*)req + 0x20;
    uint64_t unix_epoch = strtol(flighttime, NULL, 10);
    //memcpy(&unix_epoch, req + 0x40 - sizeof(unix_epoch), sizeof(unix_epoch));
    //unix_epoch = swap_uint64(unix_epoch);
    LL_NOTICE("unix_epoch=%ld, flight_number=%s", unix_epoch, flight_number);
    ret = get_flight_delay(unix_epoch, flight_number, &delay);

    LL_NOTICE("delay is %d", delay);

    *resp_data = delay;
    
    // bytes rr;
    // enc_int(rr, delay, sizeof (delay));

    /*
    uint8_t req[64];
    uint8_t raw_tx[1024];
    int raw_tx_len = sizeof ( raw_tx );
    uint8_t nonce = 1;

    ret = get_raw_signed_tx(nonce, 32, 
        1, 1, 
        req, sizeof ( req ), 
        &rr[0], 32, 
        raw_tx, &raw_tx_len);
    */

    return ret;
}

static int handler_steam_exchange(uint8_t *req, int len, int *resp_data)
{
    /*
    format[0] = encAPI[0];
    format[1] = encAPI[1];
    format[2] = ID_B;
    format[3] = T_B;
    format[4] = bytes32(LIST_I.length);
    format[5] = LIST_I[0];
    */
    int ret;
    //(void) len;

    /* handling input */
//    std::string buyer_id;  // buyer_id is 32B, each of byte takes two chars. Plus \0
    uint32_t wait_time;
    size_t item_len;
    vector<char*> items;

#ifdef VERBOSE
    dump_buf("req dump", req, len);
#endif

    // 0x00 .. 0x40
    // - encAPI: TODO: insert dec here
    unsigned char enc_api_key[64];
    memcpy(enc_api_key, req, 0x40);
    dump_buf("API KEY: ", enc_api_key, 64);

    // 0x40 .. 0x60 buyer_id
    unsigned char buyer_id[33] = {0};
    
    memcpy(buyer_id, req+0x40, 0x20);
    //LL_NOTICE("buyer id: %s", buyer_id);
    dump_buf("buyer id: ", buyer_id, 32);
    
    // 0x60 .. 0x80
    // get last 4 bytes
    memcpy(&wait_time, req + 0x80 - sizeof(uint32_t), sizeof(uint32_t));
    wait_time = swap_uint32(wait_time);

    // 0x80 .. 0xa0 - item_len
    memcpy(&item_len, req + 0xa0 - sizeof(size_t), sizeof(size_t));
    item_len = swap_uint32(item_len);//?

    // 0xa0 .. 0xc0
    for (size_t i = 0; i < item_len; i++)
    {
        items.push_back((char*) req + 0xa0 + 0x20 * i);
        LL_NOTICE("item: %s", items[i]);
    }

    if (wait_time > 3600)
        wait_time = 59;
    LL_NOTICE("waiting time: %d", wait_time);

    const char * listB[1] = {"Portal"};
    // XXX: set wait time to 1 for test purpose
//    wait_time = 1;

    int result = 0;
    ret = get_steam_transaction(listB, 1, "32884794", wait_time, "7978F8EDEF9695B57E72EC468E5781AD", &result);
    if (ret == 0 && result == 1) {
        LL_NOTICE("Found a trade");
        *resp_data = result;
        return 0;
    }

//    uncomment to simulate an real trade
//    *resp_data = 1;

    return 0;
}

int handle_request(int nonce, uint64_t id, uint64_t type, uint8_t* data, int data_len, uint8_t* raw_tx, int* raw_tx_len)
{
    int ret;
    bytes resp_data;
    int resp_data_len = 0;
/*
    printf_sgx("nonce: %d\n", nonce);
    printf_sgx("id: %llu\n", id);
    printf_sgx("type: %llu\n", type);
    printf_sgx("data len: %d\n", data_len);
    for (int i = 0; i < data_len; i++)
        printf_sgx("%u,", data[i]);
    printf_sgx("\n");
*/
    switch (type)
    {
    case TYPE_FINANCE_INFO:
        return stock_ticker_handler(nonce, id, type,
            data, data_len,
            raw_tx, raw_tx_len);
        break;
    case TYPE_FLIGHT_INS:
        {
            int found = 0;
            if (data_len != 2 * 32)
            {
                LL_CRITICAL("data_len %d*32 is not 2*32", data_len / 32);
                return -1;
            }
            ret = flight_insurance_handler(data, data_len, &found);
            if (ret != 0)
            {
                LL_CRITICAL("%s returns %d", "handler_flight_insurance", ret);
                return ret;
            }
            enc_int(resp_data, found, sizeof ( found ));
            break;
        }
    case TYPE_STEAM_EX:
        {
            int found = 0;
            if (data_len != 6 * 32)
            {
                LL_CRITICAL("data_len %d*32 is not 6*32", data_len / 32);
                return -1;
            }
            ret = handler_steam_exchange(data, data_len, &found);
            if (ret == -1)
            {
                LL_CRITICAL("%s returns %d", "handler_steam_exchange", ret);
                return -1;
            }
//            found = 1;
            enc_int(resp_data, found, sizeof ( found ));
            resp_data_len = 32;
            break;
        }
    case TYPE_CURRENT_VOTE:
        {
            double r1 = 0, r2 = 0, r3 = 0;
            long long time1, time2;

            rdtsc(&time1);
            yahoo_current("GOOG", &r1);
            rdtsc(&time2);
            LL_CRITICAL("Yahoo: %llu", time2 - time1);

            google_current("GOOG", &r3);
            rdtsc(&time1);
            LL_CRITICAL("Bloomberg: %llu", time1 - time2);

            google_current("GOOG", &r2);
            rdtsc(&time2);
            LL_CRITICAL("GOOGLE: %llu", time2 - time1);

            break;
        }
    default :
        LL_CRITICAL("Unknown request type: %d", type);
        return -1;
        break;
    }

    //sign transactions
    ret = get_raw_signed_tx(nonce, 32, id, type, data, data_len, resp_data, raw_tx, raw_tx_len);
    /*
    printf_sgx("raw tx len: %d\n", *raw_tx_len);
    for (int i = 0; i < *raw_tx_len; i++)
        printf_sgx("%u,", raw_tx[i]);
    printf_sgx("\n");
     */
    return ret;
}

