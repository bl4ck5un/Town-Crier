#if defined(__cplusplus)
extern "C" {
#endif

double get_closing_price (int month, int day, int year, char* code);

int get_flight_delay(char* date, char* time, char* flight, int* resp);

int scraper_dispatch();

#if defined(__cplusplus)
}
#endif