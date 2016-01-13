#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "io.h"
#include "TLSlib_t.h"

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
	ocall_print_string("Gulu\n");
}