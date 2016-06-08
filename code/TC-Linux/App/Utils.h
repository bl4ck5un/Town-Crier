#pragma once

#include <vector>
#include "sgx_error.h"



void fromHex(const char* src, uint8_t* target, unsigned* len);
void fromHex(const char* src, std::vector<uint8_t> & out);

//#if defined(__cplusplus)
//extern "C" {
//#endif
// Utils.cpp
void print_error_message(sgx_status_t ret);
int initialize_enclave(const char*, sgx_enclave_id_t*);

//#if defined(__cplusplus)
//}
//#endif

inline void char2hex(const uint8_t* bytes, int len, char* hex) {
    int i;
    for (i = 0; i < len; i++)
    {
        hex += sprintf(hex, "%02X", bytes[i]);
    }
    *(hex + 1) = '\0';
}

#define NONCE_FILE_NAME "nonce.bin"

inline void load_nonce(uint8_t* nonce)
{
    FILE* nonce_file = fopen(NONCE_FILE_NAME, "rb");
    fread(nonce, 32, 1, nonce_file);
    fclose(nonce_file);
}

inline void dump_nonce(uint8_t* nonce)
{
    FILE* nonce_file = fopen(NONCE_FILE_NAME, "wb");
    fwrite(nonce, 32, 1, nonce_file);
    fclose(nonce_file);
}

#if defined(_MSC_VER)
int query_sgx_status();
#endif