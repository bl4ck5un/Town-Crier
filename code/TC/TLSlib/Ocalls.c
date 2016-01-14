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
    int ret;
    va_list ap;
	char buf[BUFSIZ] = {'\0'};
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

	ocall_print_string(&ret, buf);
    return ret;
}

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen )
{
	int ret;
    (void)(data);
    ocall_mbedtls_hardware_poll(& ret, output, len, olen);
    return ret;
}