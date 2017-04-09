#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "../scrapers/stockticker.h"
#include "Log.h"

/* Test updated on 2/21/17 */
int stockticker_self_test(){
    StockTickerScraper stockTickerScraper;
    int closingPrice;
    switch(stockTickerScraper.handler(NULL, 64, &closingPrice)){
        case NO_ERROR:
            LL_INFO("closingPrice is %d", closingPrice);
            return 0;
        case WEB_ERROR:
            LL_INFO("Returned unexpected WEB_ERROR");
            return -1;
        case INVALID_PARAMS:
            LL_INFO("Returned unexpected INVALID_PARAMS");
            return 0;
    }

  	
    /* Test5: Test on flight that has not departed yet */
}