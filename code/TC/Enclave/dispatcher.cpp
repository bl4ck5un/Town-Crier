#include "dispatcher.h"
#include "Log.h"
#include "tstdio.h"

int scraper_dispatch()
{
    int ret;

    ret = flight_scraper();
    if (ret != 0)
    {
        LL_CRITICAL("yahoo_finance returned non-zero");
    }
    ret = yahoo_finance_scraper();
    if (ret != 0)
    {
        LL_CRITICAL("yahoo_finance returned non-zero");
    }
    return ret;
}