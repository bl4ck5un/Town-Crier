#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrapers.h"
#include "Log.h"
#include "tls_client.h"
//static int construct_query(const char* symbol, char** buf) {
//
//    int len;
//    char query[1000];
//    query[0] = 0;
//
//    strncat(query, "/quote/", sizeof query);
//    strncat(query, symbol, sizeof query);
//    strncat(query, ":US", sizeof query);
//
//    len = strlen(query);
//    *buf = (char*)malloc(len+1);
//    memcpy(*buf, query, len);
//    (*buf)[len] = 0;
//    return len;
//}
//
//
static double parse_response(const char* resp) {
   int len;
   double ret = 0.0;
   char * end;
   const char* temp = resp;

   while (strncmp(temp, "itemprop=\"price\"", 16) != 0) {
       temp += 1;
   }

   temp += 17;
   while (*temp != '"') {
       temp += 1;
   }

   temp += 1;
   end = (char*)temp;
   while (*end != '"') {
       end += 1;
   }
   *end = 0;
   ret = std::strtod(temp, NULL); 
   return ret;
}

int bloomberg_current(char* symbol, double* r) {
	//Null Checking
	if (symbol == NULL || r == NULL){
       LL_CRITICAL("Error: Passed in a NULL pointer\n");
       return -1;
   	}
   	/***** VARIABLE DECLARATIONS */

   	std::string query = "/quote/" + std::string(symbol) + ":US";
   	HttpRequest httpRequest("bloomberg.com", query);
    HttpsClient httpClient(httpRequest);

	/***** CONSTRUCT THE QUERY */
	try{
		HttpResponse response = httpClient.getResponse();
		*r = parse_response(response.getContent().c_str());
		return 0;		
	}
	catch(std::runtime_error &e){
		//TODO: should we not return -1 here
		LL_CRITICAL("Https error: %s", e.what());
		LL_CRITICAL("Details: %s", httpClient.getError().c_str());
		httpClient.close();
	}
	return -1;
}
//
////int main(int argc, char* argv[]) {
////    double r;
////    bloomberg_current("GOOG", &r);
////    printf("%f\n", r);
////    bloomberg_current("YHOO", &r);
////    printf("%f\n", r);
////    return 0;
////}
