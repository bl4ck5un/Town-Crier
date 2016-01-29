#include <stdio.h>
#include "App.h"

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

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

#if defined(_MSC_VER) && _MSC_VER < 1900

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
}
#endif

#endif // (_MSC_VER) && _MSC_VER < 1900