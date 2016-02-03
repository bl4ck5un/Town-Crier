#include "stdlib.h"
#include "sgx.h"
#include "sgx_urts.h"

#include <fstream>

#ifndef REPLAY_PROTECTED_SECRET_H
#define REPLAY_PROTECTED_SECRET_H

#define DFL_SECRET_LENGTH 64
class ReplayProtectedSecret
{
public:
    ReplayProtectedSecret();
    ~ReplayProtectedSecret(void);
    
    uint32_t init(std::string filename);
    uint32_t perform_function();
    uint32_t perform_function(uint8_t* stored_sealed_activity_log);
    uint32_t update_secret();
    uint32_t update_secret(uint8_t* stored_sealed_activity_log);

    uint32_t delete_secret();
    uint32_t delete_secret(uint8_t* stored_sealed_activity_log);

    uint32_t get_activity_log(uint8_t* stored_sealed_activity_log);

    
    static const uint32_t sealed_activity_log_length = DFL_SECRET_LENGTH;
private:
    uint8_t  sealed_activity_log[sealed_activity_log_length];
    sgx_enclave_id_t enclave_id;
    sgx_launch_token_t launch_token;

};

#endif
