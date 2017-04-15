#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "../scrapers/stockticker.h"
#include "Log.h"

#include "scrapers/yahoo_yql_stock.h"
#include "scrapers/yahoo_yql.h"

int stockticker_self_test() {
  string symbol = "YHOO";
  symbol.append(32 - symbol.length(), 0);

  string unix_epoch = "\x58\x3E\x8C\x80";
  unix_epoch.insert(unix_epoch.begin(), 32 - unix_epoch.length(), '\0');

  string data = symbol + unix_epoch;


  YahooYQLStock yahooYQLStock;
  int closing_price;
  err_code err = yahooYQLStock.handler((uint8_t*) data.c_str(), 32 * 4, &closing_price);
  switch (err) {
    case NO_ERROR:
      LL_INFO("closingPrice is %d", closing_price);
      return 0;
    case WEB_ERROR:
      LL_INFO("Returned unexpected WEB_ERROR");
      return -1;
    case INVALID_PARAMS:
      LL_INFO("Returned unexpected INVALID_PARAMS");
      return 0;
    default:
      return -1;
  }
}