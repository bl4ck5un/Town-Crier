#include <stdio.h>
#include <stdarg.h>

#ifndef APP_PRINT_H
#define APP_PRINT_H

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf


#if defined(__cplusplus)
extern "C" {
#endif

int ocall_print_string(const char *str);

#if defined(_MSC_VER) && _MSC_VER < 1900
int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap);
int c99_snprintf(char *outBuf, size_t size, const char *format, ...);
#endif


void dump_buf( const char *title, unsigned char *buf, size_t len );


#if defined(__cplusplus)
}
#endif

#endif APP_PRINT_H