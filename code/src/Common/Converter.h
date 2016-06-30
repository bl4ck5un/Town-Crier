//
// Created by fanz on 6/11/16.
//

#include <stdint.h>
#include <vector>

#ifndef TC_LINUX_CONVERTER_H
#define TC_LINUX_CONVERTER_H

uint32_t u32_from_b(uint8_t* b);
uint64_t u64_from_b(uint8_t* b);

int hex_to_bytes(const char *src, unsigned char *out);
long calc_b_size(const char* str);

#endif //TC_LINUX_CONVERTER_H
