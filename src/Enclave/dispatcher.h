#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
double get_closing_price (int month, int day, int year, char* code);

int get_flight_delay(uint64_t unix_epoch_time, char* flight, int* status, int* resp);

int get_steam_transaction(char** item_name_list, int item_list_len, char* other, unsigned int time_cutoff, char* key, int* resp);

int scraper_dispatch();

int google_current(char* symbol, double* r);
int yahoo_current(char* symbol, double* r);
int bloomberg_current(double* r);

#if defined(__cplusplus)
}
#endif
