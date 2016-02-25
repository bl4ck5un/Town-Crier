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


typedef struct ms_handle_request_t {
	int ms_retval;
	uint8_t* ms_nonce;
	uint64_t ms_request_id;
	uint8_t ms_request_type;
	uint8_t* ms_req;
	int ms_req_len;
	uint8_t* ms_tx;
	int* ms_len;
} ms_handle_request_t;

typedef struct ms_Test_main_t {
	int ms_retval;
} ms_Test_main_t;

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

typedef struct ms_rdtsc_t {
	long long ms_retval;
} ms_rdtsc_t;

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4200)
#endif

static sgx_status_t SGX_CDECL sgx_handle_request(void* pms)
{
	ms_handle_request_t* ms = SGX_CAST(ms_handle_request_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_nonce = ms->ms_nonce;
	size_t _len_nonce = 32;
	uint8_t* _in_nonce = NULL;
	uint8_t* _tmp_req = ms->ms_req;
	int _tmp_req_len = ms->ms_req_len;
	size_t _len_req = _tmp_req_len;
	uint8_t* _in_req = NULL;
	uint8_t* _tmp_tx = ms->ms_tx;
	size_t _len_tx = 2048 * sizeof(*_tmp_tx);
	uint8_t* _in_tx = NULL;
	int* _tmp_len = ms->ms_len;
	size_t _len_len = sizeof(*_tmp_len);
	int* _in_len = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_handle_request_t));
	CHECK_UNIQUE_POINTER(_tmp_nonce, _len_nonce);
	CHECK_UNIQUE_POINTER(_tmp_req, _len_req);
	CHECK_UNIQUE_POINTER(_tmp_tx, _len_tx);
	CHECK_UNIQUE_POINTER(_tmp_len, _len_len);

	if (_tmp_nonce != NULL) {
		_in_nonce = (uint8_t*)malloc(_len_nonce);
		if (_in_nonce == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_nonce, _tmp_nonce, _len_nonce);
	}
	if (_tmp_req != NULL) {
		_in_req = (uint8_t*)malloc(_len_req);
		if (_in_req == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_req, _tmp_req, _len_req);
	}
	if (_tmp_tx != NULL) {
		if ((_in_tx = (uint8_t*)malloc(_len_tx)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_tx, 0, _len_tx);
	}
	if (_tmp_len != NULL) {
		if ((_in_len = (int*)malloc(_len_len)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_len, 0, _len_len);
	}
	ms->ms_retval = handle_request(_in_nonce, ms->ms_request_id, ms->ms_request_type, _in_req, _tmp_req_len, _in_tx, _in_len);
err:
	if (_in_nonce) {
		memcpy(_tmp_nonce, _in_nonce, _len_nonce);
		free(_in_nonce);
	}
	if (_in_req) free(_in_req);
	if (_in_tx) {
		memcpy(_tmp_tx, _in_tx, _len_tx);
		free(_in_tx);
	}
	if (_in_len) {
		memcpy(_tmp_len, _in_len, _len_len);
		free(_in_len);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_Test_main(void* pms)
{
	ms_Test_main_t* ms = SGX_CAST(ms_Test_main_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_Test_main_t));

	ms->ms_retval = Test_main();


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_create_report(void* pms)
{
	ms_ecall_create_report_t* ms = SGX_CAST(ms_ecall_create_report_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	sgx_target_info_t* _tmp_quote_enc_info = ms->ms_quote_enc_info;
	size_t _len_quote_enc_info = sizeof(*_tmp_quote_enc_info);
	sgx_target_info_t* _in_quote_enc_info = NULL;
	sgx_report_t* _tmp_report = ms->ms_report;
	size_t _len_report = sizeof(*_tmp_report);
	sgx_report_t* _in_report = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_create_report_t));
	CHECK_UNIQUE_POINTER(_tmp_quote_enc_info, _len_quote_enc_info);
	CHECK_UNIQUE_POINTER(_tmp_report, _len_report);

	if (_tmp_quote_enc_info != NULL) {
		_in_quote_enc_info = (sgx_target_info_t*)malloc(_len_quote_enc_info);
		if (_in_quote_enc_info == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_quote_enc_info, _tmp_quote_enc_info, _len_quote_enc_info);
	}
	if (_tmp_report != NULL) {
		if ((_in_report = (sgx_report_t*)malloc(_len_report)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_report, 0, _len_report);
	}
	ms->ms_retval = ecall_create_report(_in_quote_enc_info, _in_report);
err:
	if (_in_quote_enc_info) free(_in_quote_enc_info);
	if (_in_report) {
		memcpy(_tmp_report, _in_report, _len_report);
		free(_in_report);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_time_calibrate(void* pms)
{
	ms_ecall_time_calibrate_t* ms = SGX_CAST(ms_ecall_time_calibrate_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_wtc_rsv = ms->ms_wtc_rsv;
	size_t _len_wtc_rsv = 65 * sizeof(*_tmp_wtc_rsv);
	uint8_t* _in_wtc_rsv = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_time_calibrate_t));
	CHECK_UNIQUE_POINTER(_tmp_wtc_rsv, _len_wtc_rsv);

	if (_tmp_wtc_rsv != NULL) {
		if ((_in_wtc_rsv = (uint8_t*)malloc(_len_wtc_rsv)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_wtc_rsv, 0, _len_wtc_rsv);
	}
	ms->ms_retval = ecall_time_calibrate(ms->ms_wall_clock, _in_wtc_rsv);
err:
	if (_in_wtc_rsv) {
		memcpy(_tmp_wtc_rsv, _in_wtc_rsv, _len_wtc_rsv);
		free(_in_wtc_rsv);
	}

	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* call_addr; uint8_t is_priv;} ecall_table[4];
} g_ecall_table = {
	4,
	{
		{(void*)(uintptr_t)sgx_handle_request, 0},
		{(void*)(uintptr_t)sgx_Test_main, 0},
		{(void*)(uintptr_t)sgx_ecall_create_report, 0},
		{(void*)(uintptr_t)sgx_ecall_time_calibrate, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[11][4];
} g_dyn_entry_table = {
	11,
	{
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL rdtsc(long long* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_rdtsc_t* ms;
	OCALLOC(ms, ms_rdtsc_t*, sizeof(*ms));

	status = sgx_ocall(0, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_connect(int* retval, mbedtls_net_context* ctx, const char* host, const char* port, int proto)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_host = host ? strlen(host) + 1 : 0;
	size_t _len_port = port ? strlen(port) + 1 : 0;

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
	status = sgx_ocall(1, ms);

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
	status = sgx_ocall(2, ms);

	if (retval) *retval = ms->ms_retval;
	if (ctx) memcpy((void*)ctx, ms->ms_ctx, _len_ctx);

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
	
	status = sgx_ocall(10, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
