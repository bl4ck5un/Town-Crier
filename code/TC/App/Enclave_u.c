#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_handle_request_t {
	int ms_retval;
	int ms_nonce;
	uint64_t ms_request_id;
	uint8_t ms_request_type;
	uint8_t* ms_req;
	int ms_req_len;
	uint8_t* ms_tx;
	int* ms_len;
} ms_handle_request_t;

typedef struct ms_ecall_create_report_t {
	int ms_retval;
	sgx_target_info_t* ms_quote_enc_info;
	sgx_report_t* ms_report;
} ms_ecall_create_report_t;

typedef struct ms_ecall_time_calibrate_t {
	int ms_retval;
	time_t ms_wall_clock;
	uint8_t* ms_wtc_rsv;
} ms_ecall_time_calibrate_t;

typedef struct ms_ups_tracking_t {
	int ms_retval;
	char* ms_tracking_num;
} ms_ups_tracking_t;

typedef struct ms_rdtsc_t {
	long long ms_retval;
} ms_rdtsc_t;

typedef struct ms_ocall_sleep_t {
	int ms_milisec;
} ms_ocall_sleep_t;

typedef struct ms_ocall_time_t {
	time_t ms_retval;
} ms_ocall_time_t;

typedef struct ms_ocall_mbedtls_net_connect_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	char* ms_host;
	char* ms_port;
	int ms_proto;
} ms_ocall_mbedtls_net_connect_t;

typedef struct ms_ocall_mbedtls_net_bind_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	char* ms_bind_ip;
	char* ms_port;
	int ms_proto;
} ms_ocall_mbedtls_net_bind_t;

typedef struct ms_ocall_mbedtls_net_accept_t {
	int ms_retval;
	mbedtls_net_context* ms_bind_ctx;
	mbedtls_net_context* ms_client_ctx;
	void* ms_client_ip;
	size_t ms_buf_size;
	size_t* ms_ip_len;
} ms_ocall_mbedtls_net_accept_t;

typedef struct ms_ocall_mbedtls_net_set_block_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
} ms_ocall_mbedtls_net_set_block_t;

typedef struct ms_ocall_mbedtls_net_set_nonblock_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
} ms_ocall_mbedtls_net_set_nonblock_t;

typedef struct ms_ocall_mbedtls_net_usleep_t {
	unsigned long int ms_usec;
} ms_ocall_mbedtls_net_usleep_t;

typedef struct ms_ocall_mbedtls_net_recv_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
} ms_ocall_mbedtls_net_recv_t;

typedef struct ms_ocall_mbedtls_net_send_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
} ms_ocall_mbedtls_net_send_t;

typedef struct ms_ocall_mbedtls_net_recv_timeout_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
	uint32_t ms_timeout;
} ms_ocall_mbedtls_net_recv_timeout_t;

typedef struct ms_ocall_mbedtls_net_free_t {
	mbedtls_net_context* ms_ctx;
} ms_ocall_mbedtls_net_free_t;

typedef struct ms_ocall_print_string_t {
	int ms_retval;
	char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL Enclave_rdtsc(void* pms)
{
	ms_rdtsc_t* ms = SGX_CAST(ms_rdtsc_t*, pms);
	ms->ms_retval = rdtsc();

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_sleep(void* pms)
{
	ms_ocall_sleep_t* ms = SGX_CAST(ms_ocall_sleep_t*, pms);
	ocall_sleep(ms->ms_milisec);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_time(void* pms)
{
	ms_ocall_time_t* ms = SGX_CAST(ms_ocall_time_t*, pms);
	ms->ms_retval = ocall_time();

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_connect(void* pms)
{
	ms_ocall_mbedtls_net_connect_t* ms = SGX_CAST(ms_ocall_mbedtls_net_connect_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_connect(ms->ms_ctx, (const char*)ms->ms_host, (const char*)ms->ms_port, ms->ms_proto);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_bind(void* pms)
{
	ms_ocall_mbedtls_net_bind_t* ms = SGX_CAST(ms_ocall_mbedtls_net_bind_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_bind(ms->ms_ctx, (const char*)ms->ms_bind_ip, (const char*)ms->ms_port, ms->ms_proto);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_accept(void* pms)
{
	ms_ocall_mbedtls_net_accept_t* ms = SGX_CAST(ms_ocall_mbedtls_net_accept_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_accept(ms->ms_bind_ctx, ms->ms_client_ctx, ms->ms_client_ip, ms->ms_buf_size, ms->ms_ip_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_set_block(void* pms)
{
	ms_ocall_mbedtls_net_set_block_t* ms = SGX_CAST(ms_ocall_mbedtls_net_set_block_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_set_block(ms->ms_ctx);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_set_nonblock(void* pms)
{
	ms_ocall_mbedtls_net_set_nonblock_t* ms = SGX_CAST(ms_ocall_mbedtls_net_set_nonblock_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_set_nonblock(ms->ms_ctx);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_usleep(void* pms)
{
	ms_ocall_mbedtls_net_usleep_t* ms = SGX_CAST(ms_ocall_mbedtls_net_usleep_t*, pms);
	ocall_mbedtls_net_usleep(ms->ms_usec);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_recv(void* pms)
{
	ms_ocall_mbedtls_net_recv_t* ms = SGX_CAST(ms_ocall_mbedtls_net_recv_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_recv(ms->ms_ctx, ms->ms_buf, ms->ms_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_send(void* pms)
{
	ms_ocall_mbedtls_net_send_t* ms = SGX_CAST(ms_ocall_mbedtls_net_send_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_send(ms->ms_ctx, (const unsigned char*)ms->ms_buf, ms->ms_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_recv_timeout(void* pms)
{
	ms_ocall_mbedtls_net_recv_timeout_t* ms = SGX_CAST(ms_ocall_mbedtls_net_recv_timeout_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_recv_timeout(ms->ms_ctx, ms->ms_buf, ms->ms_len, ms->ms_timeout);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_free(void* pms)
{
	ms_ocall_mbedtls_net_free_t* ms = SGX_CAST(ms_ocall_mbedtls_net_free_t*, pms);
	ocall_mbedtls_net_free(ms->ms_ctx);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ms->ms_retval = ocall_print_string((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * func_addr[14];
} ocall_table_Enclave = {
	14,
	{
		(void*)(uintptr_t)Enclave_rdtsc,
		(void*)(uintptr_t)Enclave_ocall_sleep,
		(void*)(uintptr_t)Enclave_ocall_time,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_connect,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_bind,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_accept,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_set_block,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_set_nonblock,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_usleep,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_recv,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_send,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_recv_timeout,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_free,
		(void*)(uintptr_t)Enclave_ocall_print_string,
	}
};

sgx_status_t handle_request(sgx_enclave_id_t eid, int* retval, int nonce, uint64_t request_id, uint8_t request_type, uint8_t* req, int req_len, uint8_t tx[2048], int* len)
{
	sgx_status_t status;
	ms_handle_request_t ms;
	ms.ms_nonce = nonce;
	ms.ms_request_id = request_id;
	ms.ms_request_type = request_type;
	ms.ms_req = req;
	ms.ms_req_len = req_len;
	ms.ms_tx = (uint8_t*)tx;
	ms.ms_len = len;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_create_report(sgx_enclave_id_t eid, int* retval, sgx_target_info_t* quote_enc_info, sgx_report_t* report)
{
	sgx_status_t status;
	ms_ecall_create_report_t ms;
	ms.ms_quote_enc_info = quote_enc_info;
	ms.ms_report = report;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_time_calibrate(sgx_enclave_id_t eid, int* retval, time_t wall_clock, uint8_t wtc_rsv[65])
{
	sgx_status_t status;
	ms_ecall_time_calibrate_t ms;
	ms.ms_wall_clock = wall_clock;
	ms.ms_wtc_rsv = (uint8_t*)wtc_rsv;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ups_tracking(sgx_enclave_id_t eid, int* retval, char* tracking_num)
{
	sgx_status_t status;
	ms_ups_tracking_t ms;
	ms.ms_tracking_num = tracking_num;
	status = sgx_ecall(eid, 3, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

