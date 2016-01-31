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

void dump_buf( const char *title, unsigned char *buf, size_t len )
{
    size_t i;

    printf( "%s", title );
    for( i = 0; i < len; i++ )
        printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
                       "0123456789ABCDEF" [buf[i] % 16] );
    printf( "\n" );
}

#if defined(__cplusplus)
}
#endif