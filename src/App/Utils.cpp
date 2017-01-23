#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "App.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <Log.h>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
        printf("Error: returned %x\n", ret);
}

//const char* get_error_message(sgx_status_t ret)
//{
//    
//}

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

void fromHex(const char* src, std::vector<uint8_t> & out)
{
    if (strlen(src) % 2 != 0) 
        { LL_CRITICAL("Error: input is not of even len\n");}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        out.push_back(hex2int(*src)*16 + hex2int(src[1]));
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
void fromHex(const char* src, uint8_t* target, unsigned* len)
{
    *len = 0;
    if ((strlen(src) % 2) != 0) 
        { LL_CRITICAL("Error: input is not of even len\n"); *len=0;}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        try { *(target++) = hex2int(*src)*16 + hex2int(src[1]); }
        catch (std::invalid_argument e)
            { printf("Error: can't convert %s to bytes\n", src); }
        src += 2; 
        *len = (*len)+1;
    }
    if (*len == 1 && *(target - *len) == 0) *len = 0;
}
