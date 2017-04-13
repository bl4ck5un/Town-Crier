#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "../scrapers/stockticker.h"
#include "Log.h"

#include "scrapers/yahoo_yql_stock.h"
#include "scrapers/yahoo_yql.h"

uint8_t data[] =
    {0x47, 0x4f, 0x4f, 0x47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                  0x31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                  0x32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                  0x32, 0x30, 0x31, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0};
/* Test updated on 2/21/17 */
int stockticker_self_test() {
//  StockTickerScraper stockTickerScraper;
//
//  stockTickerScraper.CreateQuery(10, 3, 2008, "GOOG");
//  StockTickerParser parser = stockTickerScraper.QueryWebsite();
//  if (parser.GetErrorCode() == WEB_ERROR) {
//    return WEB_ERROR;
//  }
//
//  LL_INFO("APPL: %d", (int) parser.GetClosingPrice());
//
//  int closingPrice;
//  switch (stockTickerScraper.handler(data, 32 * 4, &closingPrice)) {
//    case NO_ERROR:LL_INFO("closingPrice is %d", closingPrice);
//      return 0;
//    case WEB_ERROR:LL_INFO("Returned unexpected WEB_ERROR");
//      return -1;
//    case INVALID_PARAMS:LL_INFO("Returned unexpected INVALID_PARAMS");
//      return 0;
//    default:return -1;
//  }

  YahooYQLStock yahooYQLStock;
  int closing_price;
  err_code err = yahooYQLStock.handler(data, 32 * 4, &closing_price);
  switch (err) {
    case NO_ERROR:LL_INFO("closingPrice is %d", closing_price);
      return 0;
    case WEB_ERROR:LL_INFO("Returned unexpected WEB_ERROR");
      return -1;
    case INVALID_PARAMS:LL_INFO("Returned unexpected INVALID_PARAMS");
      return 0;
    default:return -1;
  }
}