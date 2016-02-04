#include <stdio.h>
#include "App.h"


#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>


#if defined(__cplusplus)
extern "C" {
#endif

/* OCall functions */
int ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    int ret = printf("%s", str);
    fflush(stdout);
    return ret;
}



#if defined(_MSC_VER) && _MSC_VER < 1900
int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}
#endif // (_MSC_VER) && _MSC_VER < 1900

void dump_buf(const char* title, void const * data, unsigned int len)
{
    unsigned int i;
    unsigned int r,c;
    
    if (!data)
	return;

    printf("%s\n", title);
    
    for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
    {
        printf("0x%04X:   ",i); /* location of first byte in line */
	
        for (c=i; c<i+8; c++) /* left half of hex dump */
	    if (c<len)
        	printf("%02X ",((unsigned char const *)data)[c]);
	    else
		printf("   "); /* pad if short line */
	
	printf("  ");
	
	for (c=i+8; c<i+16; c++) /* right half of hex dump */
	    if (c<len)
		printf("%02X ",((unsigned char const *)data)[c]);
	    else
		printf("   "); /* pad if short line */
	
	printf("   ");
	
	for (c=i; c<i+16; c++) /* ASCII dump */
	    if (c<len)
		if (((unsigned char const *)data)[c]>=32 &&
		    ((unsigned char const *)data)[c]<127)
		    printf("%c",((char const *)data)[c]);
		else
		    printf("."); /* put this for non-printables */
	    else
		printf(" "); /* pad if short line */
	
	printf("\n");
    }
}

#if defined(__cplusplus)
}
#endif