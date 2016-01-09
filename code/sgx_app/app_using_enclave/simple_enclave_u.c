#include "simple_enclave_u.h"

typedef struct ms_load_from_enc_t {
	char* ms_buf;
	size_t ms_len;
} ms_load_from_enc_t;

static const struct {
	size_t nr_ocall;
	void * func_addr[1];
} ocall_table_simple_enclave = {
	0,
	{ NULL },
};

sgx_status_t load_from_enc(sgx_enclave_id_t eid, char* buf, size_t len)
{
	sgx_status_t status;
	ms_load_from_enc_t ms;
	ms.ms_buf = buf;
	ms.ms_len = len;
	status = sgx_ecall(eid, 0, &ocall_table_simple_enclave, &ms);
	return status;
}

