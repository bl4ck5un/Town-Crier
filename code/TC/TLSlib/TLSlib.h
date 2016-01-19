#ifndef _TLSLIB_H_
#define _TLSLIB_H_

#include <stdlib.h>
#include <assert.h>

#include "Defs.h"

#define _vsnprintf vsnprintf

#if defined(__cplusplus)
extern "C" {
#endif

int printf (const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif /* !_TLSLIB_H_ */