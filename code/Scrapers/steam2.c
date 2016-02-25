#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"

char* search(char* buf, char* search_string) {
    int len = strlen(buf);
    int slen = strlen(search_string);
    char* temp = buf;
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

int construct_query1(char* key, unsigned int time, char** buf) {
    int len;
    char query[1000];
    char temp[11];

    query[0] = 0;
    temp[10] = 0;

    snprintf(temp, 11, "%d", time);


    strcat(query, "https://api.steampowered.com/IEconService/GetTradeOffers/v0001/?get_sent_offers=0&get_received_offers=1&get_descriptions=0&active_only=1&historical_only=1&key=");
    strcat(query, key);
    strcat(query, "&time_historical_cutoff=");
    strcat(query, temp);

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

int construct_query2(char* key, char* appId, char* classId, char** buf) {
    int len;
    char query[1000];
    query[0] = 0;

    strcat(query, "https://api.steampowered.com/ISteamEconomy/GetAssetClassInfo/v0001/?class_count=1&classid0=");
    strcat(query, classId);
    strcat(query, "&appid=");
    strcat(query, appId);
    strcat(query, "&key=");
    strcat(query, key);
    
    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

char* get_next_trade_with_other(char* index, char* other) {
    char* temp;
    char tempbuff[100];

    if (index == NULL) return NULL;
    if (other == NULL) return NULL;

    strncpy(tempbuff, "accountid_other\": \0", 19);
    strcat(tempbuff, other);

    temp = index+1;
    temp = search(temp, tempbuff);
    return temp;
}

int get_item_name(char * key, char* appId, char* classId, char** resp) {
    int len, ret;
    char* query = NULL;
    char* end;
    char buf[16385];
    ret = construct_query2(key, appId, classId, &query);
    if (ret < 0) return -1;
    ret = get_page_on_ssl("api.steampowered.com", query, (unsigned char*)buf, 16384);
    free(query);
    if (ret < 0) {
        /*printf("get page failed for name\n");*/
        return -1;
    }

    query = search(buf, "\"name\": \"");
    if (query == NULL) {
        return -1;
    }
    query += 9;
    end = search(query, "\"");
    if (end == NULL) {
        return -1;
    }

    len = end - query;
    *resp = (char*)malloc(len+1);
    memcpy(*resp, query, len);
    (*resp)[len] = 0;
    return 0;
}

int in_list(char** list, int len, char* name) {
    int i;
    int nlen = strlen(name);
    for (i = 0; i < len; i++) {
        if (strncmp(list[i], name, nlen) == 0) return 1;
    }
    return -1;
}

int parse_response1(char* resp, char* other, char** listB, int lenB, char* key) {
    int ret, counter, flag;
    char* index = resp, *name = NULL;
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


/* 
*/
int get_steam_transaction(char** item_name_list, int item_list_len, char* other, unsigned int time_cutoff, char* key, int* resp) {
    /***** VARIABLE DECLARATIONS */
    int ret;
    char buf[16385];
    char* query = NULL;

    //unsigned int req_time = time() + time_cutoff;
    unsigned int req_time = time_cutoff;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query1(key, req_time, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("api.steampowered.com", query, (unsigned char*)buf, 16384); 
    free(query);
    if (ret < 0) return -1;
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response1(buf, other, item_name_list, item_list_len, key);
    /***** OUTPUT */
    if (ret < 0) {
        printf("no data/bad request\n");
        return -1;
    }
    *resp = 1;
    return 0;
}


int main(int argc, char* argv[]) {
    int rc, ret;
    int time = 1456380265;
    char * listB[1] = {"Portal"};
    if (argc == 2)
        time = atoi(argv[1]);
    printf("Starting...\n");
    /* needs as input time of request, T_B time for response, key, account # ID_B, list of items L_B, account # ID_S */
    /* I'm not sure how we get time... but so long as we get it somehow...*/
    rc = get_steam_transaction(listB, 1, "32884794", time, "7978F8EDEF9695B57E72EC468E5781AD", &ret);
    printf("rc: %d, ret: %d\n", rc, ret);

    return 0;
}

