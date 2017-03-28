#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stdio.h"
#include "Log.h"
#include "stockticker.h"
#include "tls_client.h"

using namespace std;

/* Implement the stockQuery class **/
StockQuery::StockQuery(int day, int month, int year, std::string symbol){
    this->day = day; 
    this->month = month;
    this->year = year;
    this->symbol = symbol;
}

void StockQuery::SetDay(int day){
    this->day = day;
}

void StockQuery::SetMonth(int month){
    this->month = month;
}

void StockQuery::SetYear(int year){
    this->year = year;
}

void StockQuery::SetSymbol(std::string symbol){
    this->symbol = symbol;
}
//TODO: any reason we subtract the month by 1?
//TODO: A nicer C++ way to turn ints into strings?
std::string StockQuery::GetUrl(){
    char tmp[100];
    snprintf(tmp, 100,\
       "/table.csv?s=%s&a=%d&b=%d&c=%d&d=%d&e=%d&f=%d&g=d&ignore=.csv",\
       this->symbol, this->month-1, this->day, this->year, \
       this->month-1, this->day, this->year);
    std::string str(tmp);
    return str;
}

/** Implement the StockTickerScraper class **/
/* Note Still not sure why we decrease the month by 1 */
void StockTickerScraper::CreateQuery(int day, int month, int year, std::string symbol){
    this->query.SetDay(day);
    this->query.SetMonth(month-1);
    this->query.SetYear(year);
    this->query.SetSymbol(symbol);
}
/* (NOTE: Feel free to change the structure as see fit *)
/* The data is structured as follows (Feel free to change if there is a better way to structure it *:
 * 0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
 * 0x20 - 0x28 Month
 * 0x28 - 0x30 Day
 * 0x30 - 0x40 Year
 */
err_code StockTickerScraper::handler(uint8_t *req, int data_len, int *resp_data){
    //TODO
    if(data_len != 64){
        LL_CRITICAL("req_len is not 64");
        return INVALID_PARAMS;
    }

    char symbol[35] = {0};
    memcpy(symbol, req, 0x20);
    string str(symbol);

    int month = strtol((char *) req + 0x20, NULL, 10);
    int day = strtol((char *) req + 0x28, NULL, 10);
    int year = strtol((char *) req + 0x30, NULL, 10);

    
    CreateQuery(day, month, year, str);
    StockTickerParser parser = QueryWebsite();
    if(parser.GetErrorCode() == WEB_ERROR){
        return WEB_ERROR;
    }

    double closingPrice = parser.GetClosingPrice();
    *resp_data = (int) closingPrice;
    return NO_ERROR; 
}

/* Query the ichar.yahoo website, returns a StockTickerParser which can be 
    used to parse the response
*/
StockTickerParser StockTickerScraper::QueryWebsite(){
    HttpRequest httpRequest("ichart.yahoo.com",this->query.GetUrl(), NULL);
    HttpsClient httpClient(httpRequest);

    try{
        HttpResponse response = httpClient.getResponse();
        return StockTickerParser(response.getContent().c_str(), NO_ERROR);

    }catch(std::runtime_error &e){
        /* An HTTPS error has occured */
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
        httpClient.close();
        return StockTickerParser(NULL, WEB_ERROR);
    }
}


/** Implement the StockTickerParser class **/

/* Implement the constructor */
StockTickerParser::StockTickerParser(const char* resp, err_code err){
    this->rawResponse = resp; 
    this->error = err;
}

/* Returns the raw csv file */
const char* StockTickerParser::GetResponse(){
    return this->rawResponse;
}

/* Returns the error code */
err_code StockTickerParser::GetErrorCode(){
    return this->error;
}

/* returns the closing price of the stock */
double StockTickerParser::GetClosingPrice(){
    int i, len;
    unsigned char* temp = (unsigned char*)this->rawResponse;
    unsigned char* end;
    char* buf; /* hacky soln */

    if(this->rawResponse == NULL){
        LL_CRITICAL("Buf is empty!\n");
        return -1.0;
    }
    for(int i = 0; i < 10; i++){
        while(*temp != ','){
            temp += 1;
        }
        temp += 1;
    }
    end = temp;
    while(*end != ',') {
       end+=1;
    }

    len = end - temp;
    memcpy(buf, temp, len);
    buf[len] = 0;
    return std::strtod(buf, NULL);   
}