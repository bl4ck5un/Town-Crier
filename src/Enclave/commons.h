#pragma once
#include <stdint.h>
#include <vector>
#include "Log.h"
#include "encoding.h"

// home-made Big-endian long int
// NOTE: no leading zeros. Starts with useful bytes.
// size is the actual bytes that is used
// for example, bytes32 (0xAAFF) is
// AA, FF, 00, 00, .. , 00,
// size = 2

#define ROUND_TO_32(x) ((x + 31) / 32 * 32)


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


//inline void toHex(const uint8_t* bytes, int len, char* hex) {
//    int i;
//    for (i = 0; i < len; i++)
//    {
//        hex += snprintf(hex, 2, "%02X", bytes[i]);
//    }
//    *(hex + 1) = '\0';
//}

static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

inline std::string toHex(unsigned char *data, int len)
{
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}
