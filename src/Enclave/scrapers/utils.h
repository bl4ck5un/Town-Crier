//
// Created by sgx on 2/7/17.
//

#ifndef TOWN_CRIER_UTILS_H
#define TOWN_CRIER_UTILS_H
#define __STDC_FORMAT_MACROS // non needed in C, only in C++

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* #include <sstream> *>/ 

/* allows to differentiate between User error and server-side error */
enum err_code {
  NO_ERROR = 0,
  INVALID_PARAMS, /* user supplied invalid parameters to the function */
  WEB_ERROR,      /* Unable to get web request */
  UNKNOWN_ERROR,
};

/* Convers uint64 types to their string representation */
inline uint32_t swap_uint32(uint32_t num) {
  return ((num >> 24) & 0xff) | // move byte 3 to byte 0
      ((num << 8) & 0xff0000) | // move byte 1 to byte 2
      ((num >> 8) & 0xff00) | // move byte 2 to byte 1
      ((num << 24) & 0xff000000); // byte 0 to byte 3
}

inline uint64_t swap_uint64(uint64_t num) {
  return ((static_cast<uint64_t>(swap_uint32(num & 0xffffffff))) << 32) |
      (static_cast<uint64_t>(swap_uint32((num >> 32) & 0xffffffff)));
}

#endif //TOWN_CRIER_UTILS_H
