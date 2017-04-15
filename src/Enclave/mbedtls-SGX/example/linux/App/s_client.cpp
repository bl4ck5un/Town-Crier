#include <iostream>
#include <stdio.h>
#include <sgx_urts.h>

#include "Enclave_u.h"
#include "Utils.h"

using namespace std;

/* Global EID shared by multiple threads */
sgx_enclave_id_t eid = 0;

int main() {
  int ret;

#if defined(_MSC_VER)
  if (query_sgx_status() < 0) {
      cout << "sgx is not support" << endl;
      ret = -1;
      goto exit;
  }
#endif

  ret = initialize_enclave(&eid);
  if (ret != 0) {
    cerr << "failed to initialize the enclave" << endl;
    exit(-1);
  }
  printf("Enclave %lu created\n", eid);
  sgx_status_t ecall_ret = sgx_connect(eid, &ret);
  if (ecall_ret != SGX_SUCCESS) {
    cerr << "ecall failed" << endl;
    goto exit;
  }

  exit:
  sgx_destroy_enclave(eid);
  printf("Info: all enclave closed successfully.\n");
  return 0;
}
