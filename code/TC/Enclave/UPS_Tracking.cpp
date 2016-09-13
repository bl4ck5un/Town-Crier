#include "Scraper_lib.h"
#include "handlers.h"

#include "Log.h"

int ups_tracking (char* tracking_num)
{
	LL_LOG("In Enclave");
	LL_LOG("Tracking %s", tracking_num);

	char buf[3*1024];
	int ret = get_page_on_ssl("ups.com", "/", NULL, 0, (unsigned char*)buf, 3*1024); 
	LL_LOG("Done.");

	return 0;
}