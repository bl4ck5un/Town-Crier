#pragma once
#include "stdint.h"
#include <sgx_edger8r.h>


enum EX_REASONS {
    EX_GET_BLOCK_NUM,
    EX_CREATE_FILTER,
    EX_GET_FILTER_LOG,
    EX_NOTHING_TO_DO,
    EX_SEND_TRANSACTION,
    EX_HANDLE_REQ,
};

void monitor_loop(sgx_enclave_id_t eid, int nonce);
int demo_test_loop(sgx_enclave_id_t eid, uint8_t* nonce);
