#include <Debug.h>
#include "tls_client.h"
#include "scrapers/scraper_lib.h"
#include "Log.h"

#define BUFFER_LENGTH (1024*sizeof(char))
int get_page_on_ssl_self_test(){
    // XXX compare the length to CURL
    int ret;
    //Test getting something without header-lenght
    char* buf = (char*)malloc(BUFFER_LENGTH);
    ret = get_page_on_ssl("www.classe.cornell.edu","/~yuvalg/p3327/", NULL, 0, (unsigned char*)buf, BUFFER_LENGTH); 
    LL_NOTICE("buf: %s\n", buf);

    return ret;
}
