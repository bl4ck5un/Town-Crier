#include "dispatcher.h"
#include "Log.h"

#define FLIGHT

int scraper_dispatch()
{
    int ret;
    int test;
    int state; 
#ifdef FLIGHT
    ret = get_flight_delay(1477114200, "SOL361", &test);
    if (ret != 0)
    {
        LL_CRITICAL("yahoo_finance returned non-zero");
    }
    else
    {
        LL_NOTICE("Delay is %d", test);
    }
#endif

#ifdef YAHOO
    ret = yahoo_finance_scraper();
    if (ret != 0)
    {
        LL_CRITICAL("yahoo_finance returned non-zero");
    }
#endif
    return ret;
}
