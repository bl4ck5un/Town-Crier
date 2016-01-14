#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

/* sgx_ocfree() just restores the original outside stack pointer. */
#define OCALLOC(val, type, len) do {	\
	void* __tmp = sgx_ocalloc(len);	\
	if (__tmp == NULL) {	\
		sgx_ocfree();	\
		return SGX_ERROR_UNEXPECTED;\
	}			\
	(val) = (type)__tmp;	\
} while (0)


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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4200)
#endif

static sgx_status_t SGX_CDECL sgx_ecall_connect(void* pms)
{
	ms_ecall_connect_t* ms = SGX_CAST(ms_ecall_connect_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_connect_t));

	ms->ms_retval = ecall_connect();


	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* call_addr; uint8_t is_priv;} ecall_table[1];
} g_ecall_table = {
	1,
	{
		{(void*)(uintptr_t)sgx_ecall_connect, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[17][1];
} g_dyn_entry_table = {
	17,
	{
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
		{0, },
	}
};


sgx_status_t SGX_CDECL ocall_mbedtls_net_connect(int* retval, mbedtls_net_context* ctx, const char* host, const char* port, int proto)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_host = sizeof(*host);
	size_t _len_port = sizeof(*port);

	ms_ocall_mbedtls_net_connect_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_connect_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (host != NULL && sgx_is_within_enclave(host, _len_host)) {
		OCALLOC(ms->ms_host, char*, _len_host);
		memcpy((void*)ms->ms_host, host, _len_host);
	} else if (host == NULL) {
		ms->ms_host = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (port != NULL && sgx_is_within_enclave(port, _len_port)) {
		OCALLOC(ms->ms_port, char*, _len_port);
		memcpy((void*)ms->ms_port, port, _len_port);
	} else if (port == NULL) {
		ms->ms_port = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_proto = proto;
	status = sgx_ocall(0, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_bind(int* retval, mbedtls_net_context* ctx, const char* bind_ip, const char* port, int proto)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_bind_ip = sizeof(*bind_ip);
	size_t _len_port = sizeof(*port);

	ms_ocall_mbedtls_net_bind_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_bind_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (bind_ip != NULL && sgx_is_within_enclave(bind_ip, _len_bind_ip)) {
		OCALLOC(ms->ms_bind_ip, char*, _len_bind_ip);
		memcpy((void*)ms->ms_bind_ip, bind_ip, _len_bind_ip);
	} else if (bind_ip == NULL) {
		ms->ms_bind_ip = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (port != NULL && sgx_is_within_enclave(port, _len_port)) {
		OCALLOC(ms->ms_port, char*, _len_port);
		memcpy((void*)ms->ms_port, port, _len_port);
	} else if (port == NULL) {
		ms->ms_port = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_proto = proto;
	status = sgx_ocall(1, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_accept(int* retval, mbedtls_net_context* bind_ctx, mbedtls_net_context* client_ctx, void* client_ip, size_t buf_size, size_t* ip_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_bind_ctx = sizeof(*bind_ctx);
	size_t _len_client_ctx = sizeof(*client_ctx);
	size_t _len_client_ip = len;
	size_t _len_ip_len = sizeof(*ip_len);

	ms_ocall_mbedtls_net_accept_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_accept_t*, sizeof(*ms));

	if (bind_ctx != NULL && sgx_is_within_enclave(bind_ctx, _len_bind_ctx)) {
		OCALLOC(ms->ms_bind_ctx, mbedtls_net_context*, _len_bind_ctx);
		memcpy(ms->ms_bind_ctx, bind_ctx, _len_bind_ctx);
	} else if (bind_ctx == NULL) {
		ms->ms_bind_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (client_ctx != NULL && sgx_is_within_enclave(client_ctx, _len_client_ctx)) {
		OCALLOC(ms->ms_client_ctx, mbedtls_net_context*, _len_client_ctx);
		memcpy(ms->ms_client_ctx, client_ctx, _len_client_ctx);
	} else if (client_ctx == NULL) {
		ms->ms_client_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (client_ip != NULL && sgx_is_within_enclave(client_ip, _len_client_ip)) {
		OCALLOC(ms->ms_client_ip, void*, _len_client_ip);
		memcpy(ms->ms_client_ip, client_ip, _len_client_ip);
	} else if (client_ip == NULL) {
		ms->ms_client_ip = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_buf_size = buf_size;
	if (ip_len != NULL && sgx_is_within_enclave(ip_len, _len_ip_len)) {
		OCALLOC(ms->ms_ip_len, size_t*, _len_ip_len);
		memset(ms->ms_ip_len, 0, _len_ip_len);
	} else if (ip_len == NULL) {
		ms->ms_ip_len = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(2, ms);

	if (retval) *retval = ms->ms_retval;
	if (bind_ctx) memcpy((void*)bind_ctx, ms->ms_bind_ctx, _len_bind_ctx);
	if (client_ctx) memcpy((void*)client_ctx, ms->ms_client_ctx, _len_client_ctx);
	if (ip_len) memcpy((void*)ip_len, ms->ms_ip_len, _len_ip_len);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_set_block(int* retval, mbedtls_net_context* ctx)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);

	ms_ocall_mbedtls_net_set_block_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_set_block_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(3, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_set_nonblock(int* retval, mbedtls_net_context* ctx)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);

	ms_ocall_mbedtls_net_set_nonblock_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_set_nonblock_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(4, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_usleep(unsigned long int usec)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_mbedtls_net_usleep_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_usleep_t*, sizeof(*ms));

	ms->ms_usec = usec;
	status = sgx_ocall(5, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_recv(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_buf = len;

	ms_ocall_mbedtls_net_recv_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_recv_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		OCALLOC(ms->ms_buf, unsigned char*, _len_buf);
		memset(ms->ms_buf, 0, _len_buf);
	} else if (buf == NULL) {
		ms->ms_buf = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(6, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);
	if (buf) memcpy((void*)buf, ms->ms_buf, _len_buf);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_send(int* retval, mbedtls_net_context* ctx, const unsigned char* buf, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_buf = len;

	ms_ocall_mbedtls_net_send_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_send_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		OCALLOC(ms->ms_buf, unsigned char*, _len_buf);
		memcpy((void*)ms->ms_buf, buf, _len_buf);
	} else if (buf == NULL) {
		ms->ms_buf = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(7, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_recv_timeout(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len, uint32_t timeout)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_buf = len;

	ms_ocall_mbedtls_net_recv_timeout_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_recv_timeout_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		OCALLOC(ms->ms_buf, unsigned char*, _len_buf);
		memset(ms->ms_buf, 0, _len_buf);
	} else if (buf == NULL) {
		ms->ms_buf = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_len = len;
	ms->ms_timeout = timeout;
	status = sgx_ocall(8, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);
	if (buf) memcpy((void*)buf, ms->ms_buf, _len_buf);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_free(mbedtls_net_context* ctx)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);

	ms_ocall_mbedtls_net_free_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_net_free_t*, sizeof(*ms));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		OCALLOC(ms->ms_ctx, mbedtls_net_context*, _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(9, ms);

	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL mbedtls_timing_hardclock(unsigned long int* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_mbedtls_timing_hardclock_t* ms;
	OCALLOC(ms, ms_mbedtls_timing_hardclock_t*, sizeof(*ms));

	status = sgx_ocall(10, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL mbedtls_timing_get_timer(unsigned long int* retval, struct mbedtls_timing_hr_time* val, int reset)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_val = sizeof(*val);

	ms_mbedtls_timing_get_timer_t* ms;
	OCALLOC(ms, ms_mbedtls_timing_get_timer_t*, sizeof(*ms));

	if (val != NULL && sgx_is_within_enclave(val, _len_val)) {
		OCALLOC(ms->ms_val, struct mbedtls_timing_hr_time*, _len_val);
		memcpy(ms->ms_val, val, _len_val);
	} else if (val == NULL) {
		ms->ms_val = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_reset = reset;
	status = sgx_ocall(11, ms);

	if (retval) *retval = ms->ms_retval;
	if (val) memcpy((void*)val, ms->ms_val, _len_val);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL mbedtls_set_alarm(int seconds)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_mbedtls_set_alarm_t* ms;
	OCALLOC(ms, ms_mbedtls_set_alarm_t*, sizeof(*ms));

	ms->ms_seconds = seconds;
	status = sgx_ocall(12, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL mbedtls_timing_set_delay(mbedtls_timing_delay_context* data, uint32_t int_ms, uint32_t fin_ms)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_data = sizeof(*data);

	ms_mbedtls_timing_set_delay_t* ms;
	OCALLOC(ms, ms_mbedtls_timing_set_delay_t*, sizeof(*ms));

	if (data != NULL && sgx_is_within_enclave(data, _len_data)) {
		OCALLOC(ms->ms_data, mbedtls_timing_delay_context*, _len_data);
		memcpy(ms->ms_data, data, _len_data);
	} else if (data == NULL) {
		ms->ms_data = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_int_ms = int_ms;
	ms->ms_fin_ms = fin_ms;
	status = sgx_ocall(13, ms);

	if (data) memcpy((void*)data, ms->ms_data, _len_data);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL mbedtls_timing_get_delay(int* retval, mbedtls_timing_delay_context* data)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_data = sizeof(*data);

	ms_mbedtls_timing_get_delay_t* ms;
	OCALLOC(ms, ms_mbedtls_timing_get_delay_t*, sizeof(*ms));

	if (data != NULL && sgx_is_within_enclave(data, _len_data)) {
		OCALLOC(ms->ms_data, mbedtls_timing_delay_context*, _len_data);
		memcpy(ms->ms_data, data, _len_data);
	} else if (data == NULL) {
		ms->ms_data = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(14, ms);

	if (retval) *retval = ms->ms_retval;
	if (data) memcpy((void*)data, ms->ms_data, _len_data);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_hardware_poll(int* retval, unsigned char* output, size_t len, size_t* olen)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_output = len;
	size_t _len_olen = sizeof(*olen);

	ms_ocall_mbedtls_hardware_poll_t* ms;
	OCALLOC(ms, ms_ocall_mbedtls_hardware_poll_t*, sizeof(*ms));

	if (output != NULL && sgx_is_within_enclave(output, _len_output)) {
		OCALLOC(ms->ms_output, unsigned char*, _len_output);
		memset(ms->ms_output, 0, _len_output);
	} else if (output == NULL) {
		ms->ms_output = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_len = len;
	if (olen != NULL && sgx_is_within_enclave(olen, _len_olen)) {
		OCALLOC(ms->ms_olen, size_t*, _len_olen);
		memcpy(ms->ms_olen, olen, _len_olen);
	} else if (olen == NULL) {
		ms->ms_olen = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(15, ms);

	if (retval) *retval = ms->ms_retval;
	if (output) memcpy((void*)output, ms->ms_output, _len_output);
	if (olen) memcpy((void*)olen, ms->ms_olen, _len_olen);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_string(int* retval, const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms;
	OCALLOC(ms, ms_ocall_print_string_t*, sizeof(*ms));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		OCALLOC(ms->ms_str, char*, _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(16, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
