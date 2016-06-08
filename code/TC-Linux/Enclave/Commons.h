#pragma once
#include <stdint.h>
#include <vector>
#include "Log.h"
//#include "stdio.h"

// home-made Big-endian long int
// NOTE: no leading zeros. Starts with useful bytes.
// size is the actual bytes that is used
// for example, bytes32 (0xFFFF) is
// FF, FF, 00, 00, .. , 00,
// size = 2

typedef struct _b32 {uint8_t b[32]; unsigned size;} bytes32;
typedef struct _b20 {uint8_t b[20]; unsigned size;} bytes20;
typedef uint8_t byte;
typedef std::vector<uint8_t> bytes;

#define ROUND_TO_32(x) ((x + 31) / 32 * 32)


static uint8_t char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

inline void fromHex(const char* src, bytes& out)
{
    if (strlen(src) % 2 != 0) 
        { LL_CRITICAL("Error: input is not of even len\n");}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        out.push_back(char2int(*src)*16 + char2int(src[1]));
        src += 2;
    }
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
/*
    convert a hex string to a byte array
    [in]  src 
    [out] target 
    [out] len: how many bytes get written to the target
*/
inline void fromHex(const char* src, uint8_t* target, unsigned* len)
{
    *len = 0;
    if (strlen(src) > 2 && strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        try { *(target++) = char2int(*src)*16 + char2int(src[1]); } 
        catch (std::invalid_argument e)
            { printf_sgx("Error: can't convert %s to bytes\n", src); }
        src += 2; 
        *len = (*len)+1;
    }
    if (*len == 1 && *(target - *len) == 0) *len = 0;
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
