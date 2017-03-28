#include "stddef.h"

#ifndef DEBUG_H_
#define DEBUG_H_

#if defined(__cplusplus)
extern "C" {
#endif

void dump_buf( const char *title, const unsigned char *buf, size_t len );
void hexdump(const char* title, void const * data, size_t len);
void print_str_dbg(const char* title, const unsigned char* data, size_t len);

#if defined(__cplusplus)
}
#endif

#endif