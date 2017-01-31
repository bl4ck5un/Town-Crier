#ifndef APP_REMOTE_ATT_H
#define APP_REMOET_ATT_H

#include <sgx_eid.h>
#include <vector>
#include <cstdint>

std::vector<uint8_t > remote_att_init(sgx_enclave_id_t eid);
int time_calibrate(sgx_enclave_id_t eid);

#endif