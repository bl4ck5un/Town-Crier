#include "Enclave_u.h"

typedef struct ms_ecall_connect_t {
	int ms_retval;
} ms_ecall_connect_t;

typedef struct ms_ocall_mbedtls_net_connect_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	char* ms_host;
	char* ms_port;
	int ms_proto;
} ms_ocall_mbedtls_net_connect_t;

typedef struct ms_mbedtls_net_set_block_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
} ms_mbedtls_net_set_block_t;

typedef struct ms_mbedtls_net_set_nonblock_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
} ms_mbedtls_net_set_nonblock_t;

typedef struct ms_mbedtls_net_usleep_t {
	unsigned long int ms_usec;
} ms_mbedtls_net_usleep_t;

typedef struct ms_mbedtls_net_recv_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
} ms_mbedtls_net_recv_t;

typedef struct ms_mbedtls_net_send_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
} ms_mbedtls_net_send_t;

typedef struct ms_mbedtls_net_recv_timeout_t {
	int ms_retval;
	mbedtls_net_context* ms_ctx;
	unsigned char* ms_buf;
	size_t ms_len;
	uint32_t ms_timeout;
} ms_mbedtls_net_recv_timeout_t;

typedef struct ms_mbedtls_net_free_t {
	mbedtls_net_context* ms_ctx;
} ms_mbedtls_net_free_t;

typedef struct ms_mbedtls_timing_hardclock_t {
	unsigned long int ms_retval;
} ms_mbedtls_timing_hardclock_t;

typedef struct ms_mbedtls_timing_get_timer_t {
	unsigned long int ms_retval;
	struct mbedtls_timing_hr_time* ms_val;
	int ms_reset;
} ms_mbedtls_timing_get_timer_t;

typedef struct ms_mbedtls_set_alarm_t {
	int ms_seconds;
} ms_mbedtls_set_alarm_t;

typedef struct ms_mbedtls_timing_set_delay_t {
	mbedtls_timing_delay_context* ms_data;
	uint32_t ms_int_ms;
	uint32_t ms_fin_ms;
} ms_mbedtls_timing_set_delay_t;

typedef struct ms_mbedtls_timing_get_delay_t {
	int ms_retval;
	mbedtls_timing_delay_context* ms_data;
} ms_mbedtls_timing_get_delay_t;

typedef struct ms_ocall_mbedtls_hardware_poll_t {
	int ms_retval;
	unsigned char* ms_output;
	size_t ms_len;
	size_t* ms_olen;
} ms_ocall_mbedtls_hardware_poll_t;

typedef struct ms_ocall_print_string_t {
	int ms_retval;
	char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_net_connect(void* pms)
{
	ms_ocall_mbedtls_net_connect_t* ms = SGX_CAST(ms_ocall_mbedtls_net_connect_t*, pms);
	ms->ms_retval = ocall_mbedtls_net_connect(ms->ms_ctx, (const char*)ms->ms_host, (const char*)ms->ms_port, ms->ms_proto);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_set_block(void* pms)
{
	ms_mbedtls_net_set_block_t* ms = SGX_CAST(ms_mbedtls_net_set_block_t*, pms);
	ms->ms_retval = mbedtls_net_set_block(ms->ms_ctx);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_set_nonblock(void* pms)
{
	ms_mbedtls_net_set_nonblock_t* ms = SGX_CAST(ms_mbedtls_net_set_nonblock_t*, pms);
	ms->ms_retval = mbedtls_net_set_nonblock(ms->ms_ctx);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_usleep(void* pms)
{
	ms_mbedtls_net_usleep_t* ms = SGX_CAST(ms_mbedtls_net_usleep_t*, pms);
	mbedtls_net_usleep(ms->ms_usec);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_recv(void* pms)
{
	ms_mbedtls_net_recv_t* ms = SGX_CAST(ms_mbedtls_net_recv_t*, pms);
	ms->ms_retval = mbedtls_net_recv(ms->ms_ctx, ms->ms_buf, ms->ms_len);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_send(void* pms)
{
	ms_mbedtls_net_send_t* ms = SGX_CAST(ms_mbedtls_net_send_t*, pms);
	ms->ms_retval = mbedtls_net_send(ms->ms_ctx, (const unsigned char*)ms->ms_buf, ms->ms_len);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_recv_timeout(void* pms)
{
	ms_mbedtls_net_recv_timeout_t* ms = SGX_CAST(ms_mbedtls_net_recv_timeout_t*, pms);
	ms->ms_retval = mbedtls_net_recv_timeout(ms->ms_ctx, ms->ms_buf, ms->ms_len, ms->ms_timeout);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_net_free(void* pms)
{
	ms_mbedtls_net_free_t* ms = SGX_CAST(ms_mbedtls_net_free_t*, pms);
	mbedtls_net_free(ms->ms_ctx);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_timing_hardclock(void* pms)
{
	ms_mbedtls_timing_hardclock_t* ms = SGX_CAST(ms_mbedtls_timing_hardclock_t*, pms);
	ms->ms_retval = mbedtls_timing_hardclock();
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_timing_get_timer(void* pms)
{
	ms_mbedtls_timing_get_timer_t* ms = SGX_CAST(ms_mbedtls_timing_get_timer_t*, pms);
	ms->ms_retval = mbedtls_timing_get_timer(ms->ms_val, ms->ms_reset);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_set_alarm(void* pms)
{
	ms_mbedtls_set_alarm_t* ms = SGX_CAST(ms_mbedtls_set_alarm_t*, pms);
	mbedtls_set_alarm(ms->ms_seconds);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_timing_set_delay(void* pms)
{
	ms_mbedtls_timing_set_delay_t* ms = SGX_CAST(ms_mbedtls_timing_set_delay_t*, pms);
	mbedtls_timing_set_delay(ms->ms_data, ms->ms_int_ms, ms->ms_fin_ms);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_mbedtls_timing_get_delay(void* pms)
{
	ms_mbedtls_timing_get_delay_t* ms = SGX_CAST(ms_mbedtls_timing_get_delay_t*, pms);
	ms->ms_retval = mbedtls_timing_get_delay(ms->ms_data);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_mbedtls_hardware_poll(void* pms)
{
	ms_ocall_mbedtls_hardware_poll_t* ms = SGX_CAST(ms_ocall_mbedtls_hardware_poll_t*, pms);
	ms->ms_retval = ocall_mbedtls_hardware_poll(ms->ms_output, ms->ms_len, ms->ms_olen);
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
	void * func_addr[15];
} ocall_table_Enclave = {
	15,
	{
		(void*)(uintptr_t)Enclave_ocall_mbedtls_net_connect,
		(void*)(uintptr_t)Enclave_mbedtls_net_set_block,
		(void*)(uintptr_t)Enclave_mbedtls_net_set_nonblock,
		(void*)(uintptr_t)Enclave_mbedtls_net_usleep,
		(void*)(uintptr_t)Enclave_mbedtls_net_recv,
		(void*)(uintptr_t)Enclave_mbedtls_net_send,
		(void*)(uintptr_t)Enclave_mbedtls_net_recv_timeout,
		(void*)(uintptr_t)Enclave_mbedtls_net_free,
		(void*)(uintptr_t)Enclave_mbedtls_timing_hardclock,
		(void*)(uintptr_t)Enclave_mbedtls_timing_get_timer,
		(void*)(uintptr_t)Enclave_mbedtls_set_alarm,
		(void*)(uintptr_t)Enclave_mbedtls_timing_set_delay,
		(void*)(uintptr_t)Enclave_mbedtls_timing_get_delay,
		(void*)(uintptr_t)Enclave_ocall_mbedtls_hardware_poll,
		(void*)(uintptr_t)Enclave_ocall_print_string,
	}
};

sgx_status_t ecall_connect(sgx_enclave_id_t eid, int* retval)
{
	sgx_status_t status;
	ms_ecall_connect_t ms;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

