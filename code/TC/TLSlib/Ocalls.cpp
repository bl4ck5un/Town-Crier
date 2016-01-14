#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "TLSlib_t.h"  /* print_string */
#include "TLSlib.h"

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf__x(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

	int ret;
	ocall_print_string(&ret, buf);

	return ret;
}