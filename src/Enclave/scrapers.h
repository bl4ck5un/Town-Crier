#ifndef TOWN_CRIER_SCRAPERS_H
#define TOWN_CRIER_SCRAPERS_H

#include <stdint.h>
double get_closing_price (int month, int day, int year, const char* code);

int get_flight_delay(uint64_t unix_epoch_time, const char* flight, int* resp);

//int get_steam_transaction(const char** item_name_list, int item_list_len, const char* other, unsigned int time_cutoff, const char* key, int* resp);

int scraper_dispatch();
int google_current(const char* symbol, double* r);
int yahoo_current(const char* symbol, double* r);
int bloomberg_current(const char* symbol, double* r);
int ups_track(const char* tracking_num, int status);
int coinmarketcap_current(const char* symbol, double* r);
int weather_current(unsigned int zip, double* r);

#endif
