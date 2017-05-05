#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <Log.h>

#include "tls_client.h"
#include "scrapers/wolfram.h"

int wolfram_self_test() {
  WolframScraper wolframScraper;
  /* Set the type to SIMPLE */ 
  wolframScraper.set_qtype(1);

  std::string query("How far is Los Angeles from New York");
  wolframScraper.create_query(query);

  WolframQueryResult res = wolframScraper.perform_query();
  LL_INFO("status: %s", res.get_raw_data().c_str());
  
  std::string query2("population of usa");
  wolframScraper.create_query(query2);
  res = wolframScraper.perform_query();
  LL_INFO("status: %s", res.get_raw_data().c_str());
  //uspsScraper.ups_tracking("1ZE331480394808282", &resp);
  return 0;
}


