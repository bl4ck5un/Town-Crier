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
	char* ms_server;
	char* ms_port;
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

static sgx_status_t SGX_CDECL sgx_ecall_connect(void* pms)
{
	ms_ecall_connect_t* ms = SGX_CAST(ms_ecall_connect_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_server = ms->ms_server;
	size_t _len_server = _tmp_server ? strlen(_tmp_server) + 1 : 0;
	char* _in_server = NULL;
	char* _tmp_port = ms->ms_port;
	size_t _len_port = _tmp_port ? strlen(_tmp_port) + 1 : 0;
	char* _in_port = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_connect_t));
	CHECK_UNIQUE_POINTER(_tmp_server, _len_server);
	CHECK_UNIQUE_POINTER(_tmp_port, _len_port);

	if (_tmp_server != NULL) {
		_in_server = (char*)malloc(_len_server);
		if (_in_server == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_server, _tmp_server, _len_server);
		_in_server[_len_server - 1] = '\0';
	}
	if (_tmp_port != NULL) {
		_in_port = (char*)malloc(_len_port);
		if (_in_port == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_port, _tmp_port, _len_port);
		_in_port[_len_port - 1] = '\0';
	}
	ms->ms_retval = ecall_connect((const char*)_in_server, (const char*)_in_port);
err:
	if (_in_server) free((void*)_in_server);
	if (_in_port) free((void*)_in_port);

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
	uint8_t entry_table[10][1];
} g_dyn_entry_table = {
	10,
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
	}
};


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
	
	status = sgx_ocall(2, ms);

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
	
	status = sgx_ocall(3, ms);

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
	status = sgx_ocall(4, ms);


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
	status = sgx_ocall(5, ms);

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
	status = sgx_ocall(6, ms);

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
	status = sgx_ocall(7, ms);

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
	
	status = sgx_ocall(8, ms);

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
	
	status = sgx_ocall(9, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
