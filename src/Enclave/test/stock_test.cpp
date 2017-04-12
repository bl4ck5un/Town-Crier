#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "../scrapers/stockticker.h"
#include "Log.h"

uint8_t data[] = {0x47, 0x4f, 0x4f, 0x47, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                  0x31, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                  0x32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                  0x32, 0x30, 0x31, 0x30, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Test updated on 2/21/17 */
int stockticker_self_test() {
  StockTickerScraper stockTickerScraper;
  int closingPrice;
  switch (stockTickerScraper.handler(data, 32 * 4, &closingPrice)) {
    case NO_ERROR:LL_INFO("closingPrice is %d", closingPrice);
      return 0;
    case WEB_ERROR:LL_INFO("Returned unexpected WEB_ERROR");
      return -1;
    case INVALID_PARAMS:LL_INFO("Returned unexpected INVALID_PARAMS");
      return 0;
    default:return -1;
  }
}