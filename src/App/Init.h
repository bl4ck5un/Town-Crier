//
// Created by fanz on 6/11/16.
//

#include "sgx_eid.h"
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

#ifndef TC_LINUX_INIT_H
#define TC_LINUX_INIT_H

#define TOKEN_FILENAME   "towncrier.enclave.token"
#define ENCLAVE_FILENAME "enclave.signed.so"

int initialize_tc_enclave(sgx_enclave_id_t *eid);
int initialize_enclave(const char* name, sgx_enclave_id_t* eid);

#endif //TC_LINUX_INIT_H
