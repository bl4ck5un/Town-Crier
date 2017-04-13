#ifndef STEAM2_H
#define STEAM2_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "Scraper.h"
#include "tls_client.h"
#include "utils.h"
#include "Log.h"


class SteamScraper : Scraper {
private:
	std::vector<string> headers;
public:
	char* search(const char* buf, const char* search_string); 
	char* get_next_trade_with_other(char* index, const char* other);
	int get_item_name(const char * key, char* appId, char* classId, char** resp);
	int in_list(const char** list, int len, const char* name);
	int parse_response(const char *resp, const char *other, const char **listB, int lenB, const char *key);

	err_code handler(uint8_t *req, size_t len, int *resp_data);
	err_code get_steam_transaction(const char **item_name_list,
								   int item_list_len,
								   const char *buyer_id,
								   unsigned int time_cutoff,
								   const char *api_key,
								   int *resp);
};

#endif
