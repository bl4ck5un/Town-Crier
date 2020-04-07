#ifndef ENC_LOG_H
#define ENC_LOG_H

#include <stdio.h>
#include <string.h>

#include "Enclave_t.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int printf_sgx(const char *fmt, ...);

enum {
  LOG_LVL_CRITICAL,  // 0
  LOG_LVL_WARNING,   // 1
  LOG_LVL_NOTICE,    // 2
  LOG_LVL_DEBUG,     // 3
};

#ifndef LOG_BUILD_LEVEL
#ifdef NDEBUG
#define LOG_BUILD_LEVEL LOG_LVL_CRITICAL
#else
#define LOG_BUILD_LEVEL LOG_LVL_DEBUG
#endif
#endif

// The BUILD_LOG_LEVEL defines what will be compiled in the executable, in
// production it should be set to LVL_NOTICE

#ifdef __cplusplus
#define _FALSE false
#else
#define _FALSE 0
#endif

extern char log_buffer[BUFSIZ];

#define LOG_SHOULD_I(level) (level <= LOG_BUILD_LEVEL)

#define LOG(level, fmt, arg...)                             \
  do {                                                      \
    if (LOG_SHOULD_I(level)) {                              \
      snprintf(log_buffer, BUFSIZ, fmt, ##arg);             \
      ocall_logging(level, __FILE__, __LINE__, log_buffer); \
    }                                                       \
  } while (_FALSE)

#if (defined(WIN32) || defined(_WIN32))
#define LL_DEBUG(fmt, ...) LOG(LOG_LVL_DEBUG, fmt, __VA_ARGS__)
#define LL_LOG(fmt, ...) LOG(LOG_LVL_LOG, fmt, __VA_ARGS__)
#define LL_NOTICE(fmt, ...) LOG(LOG_LVL_NOTICE, fmt, __VA_ARGS__)
#define LL_WARNING(fmt, ...) LOG(LOG_LVL_WARNING, fmt, __VA_ARGS__)
#define LL_CRITICAL(fmt, ...) LOG(fmt "\n", __VA_ARGS__)
#define LL_CRITICAL(fmt, ...) LOG(LOG_LVL_CRITICAL, fmt, __VA_ARGS__)
#else
#define LL_DEBUG(fmt, arg...) LOG(LOG_LVL_DEBUG, fmt, ##arg)
#define LL_INFO(fmt, arg...) LOG(LOG_LVL_NOTICE, fmt, ##arg)
#define LL_WARNING(fmt, arg...) LOG(LOG_LVL_WARNING, fmt, ##arg)
#define LL_CRITICAL(fmt, arg...) LOG(LOG_LVL_CRITICAL, fmt, ##arg)
#endif

#ifdef __cplusplus
}
#endif

#endif
