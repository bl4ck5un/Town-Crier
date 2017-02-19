#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Log.h>
#include <vector>

#include "scrapers.h"
#include "tls_client.h"


/*
   - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
*/
#define HOST "Host: api.steampowered.com"

std::vector<string> headers;


//const char* headers[] = {HOST};
size_t header_size = 1;

char* search(const char* buf, const char* search_string) {
   int len = strlen(buf);
   int slen = strlen((char*)search_string);
   char* temp = (char*)buf;
   if (slen > len) {
       return NULL;
   }
   while(strncmp(temp, search_string, slen) != 0) {
       temp+=1;
       if (temp == buf + len - slen - 1)
           return NULL;
   }
   return temp;
}

char* get_next_trade_with_other(char* index, const char* other) {
   char* temp;
   char tempbuff[100];

   if (index == NULL) return NULL;
   if (other == NULL) return NULL;

   strncpy(tempbuff, "accountid_other\": \0", 19);
   strncat(tempbuff, other, sizeof tempbuff);

   temp = index+1;
   temp = search(temp, tempbuff);
   return temp;
}

int get_item_name(const char * key, char* appId, char* classId, char** resp) {
   	int len, ret;
   	char* end;
   	char buf[16385];

   	std::string query =  "/ISteamEconomy/GetAssetClassInfo/v0001/?class_count=1&classid0=" + \
   				std::string(classId) + \
				"&appid=" + \
				std::string(appId) + \
				"&key=" + \
   				std::string(key) + \
				" HTTP/1.1";

	HttpRequest httpRequest("api.steampowered.com", query, headers);
	HttpClient httpClient(httpRequest);
	char* query1;
   	try{
   		HttpResponse response = httpClient.getResponse();
	   	query1 = search(response.getContent().c_str(), "\"name\": \"");

   	}
   	catch(std::runtime_error& e){
   		LL_CRITICAL("Https error: %s", e.what());
   		LL_CRITICAL("Details: %s", httpClient.getError().c_str());
   		httpClient.close();
   		return -1;
   	}
   	if (query1 == NULL) {
       	return -1;
   	}
   	//Safe?
   	query1 += 9;
   	end = search(query1, "\"");
   	if (end == NULL) {
       return -1;
   	}

	len = end - query1;
	*resp = (char*)malloc(len+1);
	memcpy(*resp, query1, len);
	(*resp)[len] = 0;
	return 0;
}

int in_list(const char** list, int len, const char* name) {
   int i;
   int nlen = strlen(name);
   for (i = 0; i < len; i++) {
       if (strncmp(list[i], name, nlen) == 0) return 1;
   }
   return -1;
}

int parse_response1(const char* resp, const char* other, const char** listB, int lenB, const char* key) {
	int ret, counter, flag;
	char* index = (char*)resp;
	char* name;
	char* temp;
	char* end, *end2;
	char appId[21];
	char classId[21];

	index = get_next_trade_with_other(index, other);
	temp = index;
	while (temp != NULL) {
	end = search(temp, "confirmation_method");
	if (end == NULL) {
	  	return -1;
	}
	temp = search(temp, "trade_offer_state");
	if (temp == NULL) {
	   	return -1;
	}
	temp += 20;
	end2 = search(temp, ",");
	if (end2 == NULL || end2 > end) {
	   	return -1;
	}
	if (strncmp(temp, "3", 1) != 0) {
	   	index = get_next_trade_with_other(index, other);
	   	temp = index;
	   	continue;
	}
	temp = search(temp, "items_to_give");
	if (temp == NULL) {
	   	return -1;
	}
	if (temp > end) {
	   	index = get_next_trade_with_other(index, other);
	   	temp = index;
	   	continue;
	}
	temp = search(temp, "appid\": \"");
	if (temp == NULL) {
	   	return -1;
	}

	temp += 9;
	/* for all items in this trade */
	counter = 0;
	flag = 0;
	while (temp != NULL && temp < end) {
	   	end2 = search(temp, "\"");
	   	if (end2 == NULL) {
	       	return -1;
	   	}
	   	strncpy(appId, temp, (int)(end2-temp));
	   	appId[(int)(end2-temp)] = 0;

	   	temp = search(temp, "classid\": \"");
	   	if (temp == NULL) {
	       	return -1;
	   	}
	   	temp += 11;
	   	end2 = search(temp, "\"");
	   	if (end2 == NULL) {
	       	return -1;
	   	}
	   	strncpy(classId, temp, (int)(end2-temp));
	   	classId[(int)(end2-temp)] = 0;

	   /* get name by 2nd request */
	   	ret = get_item_name(key, appId, classId, &name);
	   	if (ret < 0) {
	       	return -1;
	   	}
	   	ret = in_list(listB, lenB, name);
	   	free(name);
	   	if (ret < 0) {
	       	flag = 1;
	       	break;
	   	}
	   	counter += 1;

	   /* continue loop */
	   	temp = search(temp, "appid\": \"");
	}
	/* we found a matching trade */
	if (counter == lenB && flag == 0) {
	   	return 0;
	}
	/* continue outer loop over all trades */
	index = get_next_trade_with_other(index, other);
	temp = index;
	}
	/*printf("SEARCHED ALL TRADES: NOT FOUND\n");*/
	return -1;
}

//
///*
//*/
int get_steam_transaction(const char** item_name_list, int item_list_len, const char* other, unsigned int time_cutoff, const char* key, int* resp) {
	int ret;
	int i;
	char buf[16385];
	char* query = NULL;

	time_t time1, time2;
	headers.push_back(HOST);



	for (i = 0; i < 20; i++) {
	   /*
	   ocall_time(&time1);
	   ocall_sleep(10 * 1000);
	   ocall_time(&time2);
	   */

	   LL_NOTICE("%lld seconds passed", time2 - time1);

	   unsigned int req_time = (unsigned int) time1 - (30*60);
	   char tmp_req_time[10];
	   snprintf(tmp_req_time, sizeof(tmp_req_time), "%u", req_time);

	   // reference query
	   // https://api.steampowered.com/IEconService/GetTradeOffers/v0001/?get_sent_offers=1&get_received_offers=0&get_descriptions=0&active_only=1&historical_only=1&key=7978F8EDEF9695B57E72EC468E5781AD&time_historical_cutoff=1355220300

   		std::string query = "/IEconService/GetTradeOffers/v0001/?get_sent_offers=0&get_received_offers=1&get_descriptions=0&active_only=1&historical_only=1&key=" + \
   					std::string(key) + \
   					"&time_historical_cutoff=" + \
   					std::string(tmp_req_time) + \
   					" HTTP/1.1";
		
		HttpRequest httpRequest("api.steampowered.com", query, headers);
    	HttpClient httpClient(httpRequest);

	   	try{
	   		HttpResponse response = httpClient.getResponse();
			ret = parse_response1(response.getContent().c_str(), other, item_name_list, item_list_len, key);
	   	}
	   	catch (std::runtime_error& e){
	        LL_CRITICAL("Https error: %s", e.what());
	        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
	        httpClient.close();	   		
	   		return -1;
	   	}

		if (ret < 0) {
	       LL_CRITICAL("Found no trade");
	       *resp = 0;
	       return 0;
	   	}
	   	else {
	       *resp = 1;
	       return 0;
	   	}
	}
	*resp = 0;
    return 0;
}
//
///*
//int main(int argc, char* argv[]) {
//    int rc, ret;
//    char * listB[1] = {"Portal"};
//    char * test2[2] = {"Dark Ranger's Headdress", "Death Shadow Bow"};
//    printf("Starting...\n");
//    // needs as input time of request, T_B time for response, key, account # ID_B, list of items L_B, account # ID_S
//    // I'm not sure how we get time... but so long as we get it somehow...
//    rc = get_steam_transaction(listB, 1, "32884794", 1456380265, "7978F8EDEF9695B57E72EC468E5781AD", &ret);
//    printf("%d, %d\n", rc, ret);
//    //rc should be 0, ret should be 1
//
//    return 0;
//}
//
//*/
