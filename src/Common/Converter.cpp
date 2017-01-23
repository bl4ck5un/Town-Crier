//
// Created by fanz on 6/11/16.
//

#include "Converter.h"
#include <stdexcept>

#include <boost/algorithm/hex.hpp>
#include <iostream>

using namespace std;


uint32_t u32_from_b(uint8_t* b) {
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

// @Deprecated
//int hex_to_bytes(const char *str, unsigned char *b) {
//    if (str == NULL) { return -1; }
//    if (strlen(str) % 2 != 0)
//    {
//        LL_CRITICAL("Error: input is not of even len");
//        return -1;
//    }
//    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {str += 2;}
//    int i = 0;
//    while(*str && str[1])
//    {
//        b[i] = hex2int(str[0])*16 + hex2int(str[1]);
//        str += 2;
//        i++;
//    }
//
//    return 0;
//}

void hexToBuffer(const string &str, unsigned char *buffer, size_t bufSize) {
  if (buffer == nullptr)
    throw invalid_argument("buffer is null");
  if (str.size() == 0)
    return;

  auto offset = (str.compare(0, 2, "0x") == 0) ? 2 : 0;
  if ((str.size() - offset) / 2 > bufSize) {
    throw invalid_argument("buffer is too small");
  }
  boost::algorithm::unhex(str.begin() + offset, str.end(), buffer);
}

string bufferToHex(const unsigned char *buffer, size_t bufSize, bool prefix) {
  string hex;
  if (prefix) {
    hex += "0x";
  }
  try {
    boost::algorithm::hex(buffer, buffer + bufSize, back_inserter(hex));
    return hex;
  }
  catch (boost::algorithm::hex_decode_error &e) {
    std::cerr << boost::diagnostic_information(e) << std::endl;
    throw invalid_argument(boost::diagnostic_information(e, false));
  }
}