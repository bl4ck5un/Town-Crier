#pragma once

#ifndef _TLSLIB_DEFS_H_
#define _TLSLIB_DEFS_H_

#if defined(_MSC_VER)
/* The following macros are MSVC only */

# define MKOCALL_INT(fname, ...) int ret; \
	ocall_ ## fname ## (&ret, ## __VA_ARGS__ ## ); \
	return ret;\

# define MKOCALL_VOID(fname, ...) ocall_ ## fname ## (## __VA_ARGS__ ## );

#elif defined(__GNUC__)
/* The following macros are GCC only */


#endif /* __GNUC__ */

#endif /* !_SGX_DEFS_H_ */