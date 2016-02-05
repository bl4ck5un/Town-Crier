#if defined(__cplusplus)
extern "C" {
#endif

int yahoo_finance_scraper ();

int get_flight_delay(char* date, char* time, char* flight, int* resp);

int scraper_dispatch();

#if defined(__cplusplus)
}
#endif