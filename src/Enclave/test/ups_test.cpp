#include <stdio.h>
#include <stdlib.h>

#include "scrapers/UPS_Tracking.h"
#include <string>


int ups_self_test() {
  //std::string example_json("{\"id\": \"trk_c8e0edb5bb284caa934a0d3db23a148z\",\"tracking_details\": [{\"object\": \"TrackingDetail\",\"message\": \"Shipping Label Created\",\"status\": \"pre_transit\",\"datetime\": \"2015-12-31T15:58:00Z\",\"source\": \"USPS\",\"tracking_location\": {\"object\": \"TrackingLocation\",\"city\": \"FOUNTAIN VALLEY\",\"state\": \"CA\",\"country\": null,\"zip\": \"92708\"}},{\"object\": \"TrackingDetail\",\"message\": \"Arrived at Post Office\",\"status\": \"in_transit\",\"datetime\": \"2016-01-07T06:58:00Z\",\"source\": \"USPS\",\"tracking_location\": {\"object\": \"TrackingLocation\",\"city\": \"FOUNTAIN VALLEY\",\"state\": \"CA\",\"country\": null,\"zip\": \"92728\"}}]}");

  USPSScraper uspsScraper;
  int resp;
  //std::string status = uspsScraper.parse_response(example_json);
  //LL_INFO("status: %s", status.c_str());
  uspsScraper.ups_tracking("EZ1000000001,", &resp);
  return 0;
}


