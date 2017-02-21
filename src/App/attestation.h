#ifndef APP_REMOTE_ATT_H
#define APP_REMOET_ATT_H

#include <sgx_eid.h>
#include <vector>
#include <stdexcept>
#include <cstdint>

std::vector<uint8_t > get_attestation(sgx_enclave_id_t eid) throw(std::runtime_error);
int time_calibrate(sgx_enclave_id_t eid);

#endif