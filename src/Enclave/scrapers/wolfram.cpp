/* Define wolfrasm scraper constant */
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <Log.h>

#include "wolfram.h"
#include "utils.h"
#include "tls_client.h"

/*** Implement the WolframQueryResults class ***/
using namespace std;

/* WolframResults constructor */
WolframQueryResult::WolframQueryResult(string xml){
	this->xml = xml;
}

/* Return a pointer to the raw xml file */
string WolframQueryResult::get_raw_data(){
	return this->xml;
}

/*** Implement the WolfRamScraper class ***/

// WolframScraper::WolframScraper(int qtype){
//   this->wolframQueryType = qType; 
// }

void WolframScraper::create_query(std::string query){

  char* tmp = (char*)query.c_str();
  for(int i = 0; i < query.size(); i++){
    if (tmp[i] == ' '){
      tmp[i] = '+';
    }
  }
  string newQ(tmp);

  this->url = "/v1/result?appid=" + this->APPID + "&i=" + newQ;
  LL_INFO("url is : %s", this->url.c_str()); 
}

err_code WolframScraper::handler(const uint8_t *req, size_t data_len, int *resp_data){
  if (data_len != 2*32){
    LL_CRITICAL("Data len %d*2 does not equal 2*32", data_len);
    return INVALID_PARAMS;
  }
  //Parse the request type followed by the query
  char qType[32] = {0};
  char query[32] = {0};

  memcpy(qType, req, 0x20);
  memcpy(query, req + 0x20, 0x20);

  if(qType[0] == '1'){
    this->wolframQueryType = SIMPLE;
  }
  else{
    LL_CRITICAL("More complicated queries not supported yet!");
    return INVALID_PARAMS;
  }

  /* Construct the query */
  string tmp(query);
  create_query(tmp);
  WolframQueryResult results = perform_query();
  if(this->wolframQueryType == SIMPLE){
    //*resp_data = (int)(results.get_raw_data().c_str());
    *resp_data = -1;
  }
  return INVALID_PARAMS; 
}

/* Function that performs the HTTPS request and return the xml file */
WolframQueryResult WolframScraper::perform_query(){
  HttpRequest httpRequest(this->HOST, this->url, true);
  HttpsClient httpClient(httpRequest);  

	try{
		HttpResponse resp = httpClient.getResponse();
    //*status = NO_ERROR;
    WolframQueryResult wolframQueryResult(resp.getContent());
    return wolframQueryResult;
	}
	catch(std::runtime_error &e){
		LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    //*status = WEB_ERROR;
	}
  /* Hacky: add exception handling */
  WolframQueryResult tmp("");
  return tmp;
}

void WolframScraper::set_qtype(int type){
  switch (type){
    case 1:
      this->wolframQueryType = SIMPLE;
      break;
    default:
      this->wolframQueryType = GENERAL;
      break;
  }
}
