#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Log.h>

#include "scrapers.h"
#include "utils.h"
#include "tls_client.h"
#include "../../Common/Constants.h"
//#include <iostream>

using namespace std;
/* Define USPS Scraper specific constants */
#define USPS_API "http://production.shippingapis.com"
#define USERID "063CORNE4274"

/* Define valid states of packages */ 
enum usps_state{
	PACKAGE_NOT_FOUND =0,
	ORDER_PROCESSED,
	SHIPPED,
	IN_TRANSIT,
	OUT_FOR_DELIVERY,
	DELIVERED,
};

class USPSScraper{
public:
	/* Method used to handle any UPSP data request */
	virtual err_code handler(uint8_t *req, int len, int *resp_data){
		//Parse the raw array to get the required params
		return NO_ERROR;
	}
	

	int ups_tracking (const char* tracking_num, int* status){
		
		if (tracking_num == NULL){
			LL_CRITICAL("Error: Passed in NULL Pointer");
			*status = -1;
			return -1;
		}

		/* Build the query */ 
		std::string query = "/ShippingAPI.dll?API=TrackV2&XML=<TrackRequest USERID=063CORNE4274><TrackID ID=\"" + std::string(tracking_num) + "\"></TrackID></TrackRequest>";

		HttpRequest httpRequest(USPS_API, query);
		HttpsClient httpClient(httpRequest);
		std::string result; 
		try{
			HttpResponse response = httpClient.getResponse();
	        result = parse_response(response.getContent().c_str());
		}
		catch (std::runtime_error& e){
	        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
	        httpClient.close();
	        *status = -1;
	        return -1;		
		}

		// return an int according to the result. E.g. 1 for delivered, etc.
	    if(result.compare("Package not found") == 0){
	    	*status = PACKAGE_NOT_FOUND;
	    	return 0;
	    }
	    if(result.compare("Delivered") == 0){
	        *status = DELIVERED;
	        return 0;
	    }
	    if(result.compare("Order processed") == 0){
	        *status = ORDER_PROCESSED;
	        return 0;
	    }
	    if(result.compare("Shipped") == 0){
	        *status = SHIPPED;
	        return 0;
	    }
	    if(result.compare("In transit") == 0){
	        *status = IN_TRANSIT;
	        return 0;
	    }
	    if(result.compare("Out for delivery") == 0){
	        *status = OUT_FOR_DELIVERY;
	        return 0;
	    }
	    else{
	    	LL_CRITICAL("failed to get information\n");
	        return -1;
	    }
	}

private:
	std::string parse_response(const char* resp){

		//char* end;
		char* tmp = (char*)resp; 
		std::string buf_string(resp); 
		//cout << buf_string << "\n";
		
		std::size_t pos = buf_string.find("id=\"tt_spStatus\"");
		if (pos == std::string::npos){
			std::string no_pkg = "Package not found";
			return no_pkg;
		}

		//std::string new_tmp = tmp.substr(pos, pos + 20);
		//printf("test\n");
		std::size_t start = pos + 41; 
		pos += 41;
		while(tmp[pos] != '\t'){
			pos += 1;
		}
		std::size_t end = pos - 1;
		std::string token = buf_string.substr(start, end-start);

		return token;
	}

};
/* Code used for testing
int main(){
	char trackID[] = "1Z1aasdfa581202007873";
	char* tID = trackID;
	std::string status; 
	ups_current(trackID, &status);
	cout << status << "\n";
	return 0; 

}
*/ 
