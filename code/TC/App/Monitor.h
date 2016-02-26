#pragma once
#include "stdint.h"
#include <sgx_edger8r.h>

int monitor_loop(sgx_enclave_id_t eid, uint8_t* nonce);
int demo_test_loop(sgx_enclave_id_t eid, uint8_t* nonce);