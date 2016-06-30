//
// Created by fanz on 6/11/16.
//

#include "Converter.h"
#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <string.h>
#include "Log.h"

using namespace std;


uint32_t u32_from_b(uint8_t* b) {
    // take 4 bytes starting from b
    // and change it to little endian
    uint32_t r = *(uint32_t*)b;
    return __builtin_bswap32(r);
}

uint64_t u64_from_b(uint8_t* b) {
    uint64_t r = *(uint64_t*)b;
    return __builtin_bswap64(r);
}

static uint8_t hex2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}


int hex_to_bytes(const char *str, unsigned char *b) {
    if (str == NULL) { return -1; }
    if (strlen(str) % 2 != 0)
    {
        LL_CRITICAL("Error: input is not of even len");
        return -1;
    }
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {str += 2;}
    int i = 0;
    while(*str && str[1])
    {
        b[i] = hex2int(str[0])*16 + hex2int(str[1]);
        str += 2;
        i++;
    }

    return 0;
}

long calc_b_size(const char* str) {
    if (str == NULL) {return -1;}
    long len = strlen(str);
    if (len % 2 != 0) {
        LL_CRITICAL("Error: input is of odd len %ld", len);
        return -1;
    }
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        return len / 2 -1;
    }
    return len / 2;
}