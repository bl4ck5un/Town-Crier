#include "https.h"
#include <string>
#include <iostream>

using namespace std;

int main () 
{
    size_t buf_size = 100*1024;
    unsigned char buf [100*1024];
    int ret = get_page_on_ssl("https://www.ups.com", "/", NULL, 0, buf, buf_size);

    
    // parse buf

    return ret;
}
