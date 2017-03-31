#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrapers.h"
#include "Log.h"
#include "tls_client.h"

#include "macros.h"

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
// WARNING: bloomberg is deprecated because the web page is too verbose.
DEPRECATED(int bloomberg_current(const char* symbol, double* r));
int bloomberg_current(const char* symbol, double* r) {
  /*
	//Null Checking
	if (symbol == NULL || r == NULL){
       LL_CRITICAL("Error: Passed in a NULL pointer");
       return -1;
   	}
   	std::string query = "/quote/" + std::string(symbol) + ":US";
   	HttpRequest httpRequest("www.bloomberg.com", query, true);
    HttpsClient httpClient(httpRequest);

	try{
        HttpResponse response = httpClient.getResponse();
      LL_DEBUG("get response %s", response.getContent().c_str());
		 *r = parse_response(response.getContent().c_str());
		return 0;		
	}
	catch(std::runtime_error &e){
		LL_CRITICAL("Https error: %s", e.what());
		LL_CRITICAL("Details: %s", httpClient.getError().c_str());
		httpClient.close();
	}
	return -1;
  */
  return 0;
}