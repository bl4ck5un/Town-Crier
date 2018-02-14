//
// Created by fanz on 2/14/18.
//

#ifndef TOWN_CRIER_MACROS_H
#define TOWN_CRIER_MACROS_H

#if defined(IN_ENCLAVE) && defined(__cplusplus)
#include "log.h"

#define NO_THROW(x) \
  try { \
  do { \
    x \
  } while (false); \
  } \
  catch (const std::exception& e) { \
    LL_CRITICAL("Exception: %s", e.what()); \
  } \
  catch (...) \
  { \
    LL_CRITICAL("unknown exception happend"); \
  }

#define NO_THROW_RET(x) \
  try { \
  do { \
    x \
  } while (false); \
  } \
  catch (const std::exception& e) { \
    LL_CRITICAL("Exception: %s", e.what()); \
    return -1; \
  } \
  catch (...) \
  { \
    LL_CRITICAL("unknown exception happend"); \
    return -1; \
  }
#endif

#endif //TOWN_CRIER_MACROS_H
