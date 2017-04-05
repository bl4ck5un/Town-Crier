//
// Created by fanz on 4/4/17.
//

#ifndef TOWN_CRIER_KEY_UTILS_H
#define TOWN_CRIER_KEY_UTILS_H

#include <sgx_eid.h>
#include <string>

using std::string;

string unseal_key(sgx_enclave_id_t eid, string sealed_key);

#endif //TOWN_CRIER_KEY_UTILS_H
