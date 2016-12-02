#include "Debug.h"
#include "string.h"
#include "Log.h"

#define MIN(x,y) (x < y ? x : y)


void dump_buf( const char *title, const unsigned char *buf, size_t len )
{
    if (log_run_level >= LOG_LVL_DEBUG)
        hexdump(title, buf, len);
}

void print_str_dbg(const char* title, const unsigned char* data, size_t len)
{
    unsigned char buf[1024 + 1] = {0};
    if (!data) {
        return;
    }
    if (len == 0) {
        LL_CRITICAL("zero length");
        return;
    }

    long remaining = len, tp = 0;

    printf_sgx("%s: ", title);

    while (remaining > 0) {
        tp = MIN(1024, remaining);

        memcpy(buf, data, tp);
        buf[tp] = '\0'; // just to make sure
        printf_sgx("%s", buf);
        remaining -= 1024;
        data += 1024;
    }
	printf_sgx("\n");
}

void hexdump(const char* title, void const * data, size_t len)
{
    unsigned int i;
    unsigned int r,c;
    
    if (!data)
	return;

    printf_sgx("%s\n", title);
    
    for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
    {
        printf_sgx("0x%04X:   ",i); /* location of first byte in line */
	
        for (c=i; c<i+8; c++) /* left half of hex dump */
	    if (c<len)
        	printf_sgx("%02X ",((unsigned char const *)data)[c]);
	    else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("  ");
	
	for (c=i+8; c<i+16; c++) /* right half of hex dump */
	    if (c<len)
		printf_sgx("%02X ",((unsigned char const *)data)[c]); else
		printf_sgx("   "); /* pad if short line */
	
	printf_sgx("   ");
	
	for (c=i; c<i+16; c++) /* ASCII dump */
	    if (c<len)
		if (((unsigned char const *)data)[c]>=32 &&
		    ((unsigned char const *)data)[c]<127)
		    printf_sgx("%c",((char const *)data)[c]);
		else
		    printf_sgx("."); /* put this for non-printables */
	    else
		printf_sgx(" "); /* pad if short line */
	
	printf_sgx("\n");
    }
}
