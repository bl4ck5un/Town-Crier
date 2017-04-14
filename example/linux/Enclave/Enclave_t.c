#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
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


typedef struct ms_sgx_connect_t {
	int ms_retval;
} ms_sgx_connect_t;

typedef struct ms_sgx_accept_t {
	int ms_retval;
} ms_sgx_accept_t;



typedef struct ms_ssl_conn_handle_t {
	long int ms_thread_id;
	thread_info_t* ms_thread_info;
} ms_ssl_conn_handle_t;


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

typedef struct ms_sgx_oc_cpuidex_t {
	int* ms_cpuinfo;
	int ms_leaf;
	int ms_subleaf;
} ms_sgx_oc_cpuidex_t;

typedef struct ms_sgx_thread_wait_untrusted_event_ocall_t {
	int ms_retval;
	void* ms_self;
} ms_sgx_thread_wait_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_set_untrusted_event_ocall_t {
	int ms_retval;
	void* ms_waiter;
} ms_sgx_thread_set_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_setwait_untrusted_events_ocall_t {
	int ms_retval;
	void* ms_waiter;
	void* ms_self;
} ms_sgx_thread_setwait_untrusted_events_ocall_t;

typedef struct ms_sgx_thread_set_multiple_untrusted_events_ocall_t {
	int ms_retval;
	void** ms_waiters;
	size_t ms_total;
} ms_sgx_thread_set_multiple_untrusted_events_ocall_t;

static sgx_status_t SGX_CDECL sgx_sgx_connect(void* pms)
{
	ms_sgx_connect_t* ms = SGX_CAST(ms_sgx_connect_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_sgx_connect_t));

	ms->ms_retval = sgx_connect();


	return status;
}

static sgx_status_t SGX_CDECL sgx_sgx_accept(void* pms)
{
	ms_sgx_accept_t* ms = SGX_CAST(ms_sgx_accept_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_sgx_accept_t));

	ms->ms_retval = sgx_accept();


	return status;
}

static sgx_status_t SGX_CDECL sgx_ssl_conn_init(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ssl_conn_init();
	return status;
}

static sgx_status_t SGX_CDECL sgx_ssl_conn_teardown(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ssl_conn_teardown();
	return status;
}

static sgx_status_t SGX_CDECL sgx_ssl_conn_handle(void* pms)
{
	ms_ssl_conn_handle_t* ms = SGX_CAST(ms_ssl_conn_handle_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	thread_info_t* _tmp_thread_info = ms->ms_thread_info;
	size_t _len_thread_info = sizeof(*_tmp_thread_info);
	thread_info_t* _in_thread_info = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_ssl_conn_handle_t));
	CHECK_UNIQUE_POINTER(_tmp_thread_info, _len_thread_info);

	if (_tmp_thread_info != NULL) {
		_in_thread_info = (thread_info_t*)malloc(_len_thread_info);
		if (_in_thread_info == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_thread_info, _tmp_thread_info, _len_thread_info);
	}
	ssl_conn_handle(ms->ms_thread_id, _in_thread_info);
err:
	if (_in_thread_info) {
		memcpy(_tmp_thread_info, _in_thread_info, _len_thread_info);
		free(_in_thread_info);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_dummy(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	dummy();
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[6];
} g_ecall_table = {
	6,
	{
		{(void*)(uintptr_t)sgx_sgx_connect, 0},
		{(void*)(uintptr_t)sgx_sgx_accept, 0},
		{(void*)(uintptr_t)sgx_ssl_conn_init, 0},
		{(void*)(uintptr_t)sgx_ssl_conn_teardown, 0},
		{(void*)(uintptr_t)sgx_ssl_conn_handle, 0},
		{(void*)(uintptr_t)sgx_dummy, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[16][6];
} g_dyn_entry_table = {
	16,
	{
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_mbedtls_net_connect(int* retval, mbedtls_net_context* ctx, const char* host, const char* port, int proto)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);
	size_t _len_host = host ? strlen(host) + 1 : 0;
	size_t _len_port = port ? strlen(port) + 1 : 0;

	ms_ocall_mbedtls_net_connect_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_connect_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;
	ocalloc_size += (host != NULL && sgx_is_within_enclave(host, _len_host)) ? _len_host : 0;
	ocalloc_size += (port != NULL && sgx_is_within_enclave(port, _len_port)) ? _len_port : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_connect_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_connect_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (host != NULL && sgx_is_within_enclave(host, _len_host)) {
		ms->ms_host = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_host);
		memcpy((void*)ms->ms_host, host, _len_host);
	} else if (host == NULL) {
		ms->ms_host = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (port != NULL && sgx_is_within_enclave(port, _len_port)) {
		ms->ms_port = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_port);
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
	size_t _len_bind_ip = bind_ip ? strlen(bind_ip) + 1 : 0;
	size_t _len_port = port ? strlen(port) + 1 : 0;

	ms_ocall_mbedtls_net_bind_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_bind_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;
	ocalloc_size += (bind_ip != NULL && sgx_is_within_enclave(bind_ip, _len_bind_ip)) ? _len_bind_ip : 0;
	ocalloc_size += (port != NULL && sgx_is_within_enclave(port, _len_port)) ? _len_port : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_bind_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_bind_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
		memset(ms->ms_ctx, 0, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (bind_ip != NULL && sgx_is_within_enclave(bind_ip, _len_bind_ip)) {
		ms->ms_bind_ip = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_bind_ip);
		memcpy((void*)ms->ms_bind_ip, bind_ip, _len_bind_ip);
	} else if (bind_ip == NULL) {
		ms->ms_bind_ip = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (port != NULL && sgx_is_within_enclave(port, _len_port)) {
		ms->ms_port = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_port);
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
	size_t _len_client_ip = buf_size;
	size_t _len_ip_len = sizeof(*ip_len);

	ms_ocall_mbedtls_net_accept_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_accept_t);
	void *__tmp = NULL;

	ocalloc_size += (bind_ctx != NULL && sgx_is_within_enclave(bind_ctx, _len_bind_ctx)) ? _len_bind_ctx : 0;
	ocalloc_size += (client_ctx != NULL && sgx_is_within_enclave(client_ctx, _len_client_ctx)) ? _len_client_ctx : 0;
	ocalloc_size += (client_ip != NULL && sgx_is_within_enclave(client_ip, _len_client_ip)) ? _len_client_ip : 0;
	ocalloc_size += (ip_len != NULL && sgx_is_within_enclave(ip_len, _len_ip_len)) ? _len_ip_len : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_accept_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_accept_t));

	if (bind_ctx != NULL && sgx_is_within_enclave(bind_ctx, _len_bind_ctx)) {
		ms->ms_bind_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_bind_ctx);
		memcpy(ms->ms_bind_ctx, bind_ctx, _len_bind_ctx);
	} else if (bind_ctx == NULL) {
		ms->ms_bind_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (client_ctx != NULL && sgx_is_within_enclave(client_ctx, _len_client_ctx)) {
		ms->ms_client_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_client_ctx);
		memset(ms->ms_client_ctx, 0, _len_client_ctx);
	} else if (client_ctx == NULL) {
		ms->ms_client_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (client_ip != NULL && sgx_is_within_enclave(client_ip, _len_client_ip)) {
		ms->ms_client_ip = (void*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_client_ip);
		memset(ms->ms_client_ip, 0, _len_client_ip);
	} else if (client_ip == NULL) {
		ms->ms_client_ip = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_buf_size = buf_size;
	if (ip_len != NULL && sgx_is_within_enclave(ip_len, _len_ip_len)) {
		ms->ms_ip_len = (size_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ip_len);
		memset(ms->ms_ip_len, 0, _len_ip_len);
	} else if (ip_len == NULL) {
		ms->ms_ip_len = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(2, ms);

	if (retval) *retval = ms->ms_retval;
	if (client_ctx) memcpy((void*)client_ctx, ms->ms_client_ctx, _len_client_ctx);
	if (client_ip) memcpy((void*)client_ip, ms->ms_client_ip, _len_client_ip);
	if (ip_len) memcpy((void*)ip_len, ms->ms_ip_len, _len_ip_len);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mbedtls_net_set_block(int* retval, mbedtls_net_context* ctx)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ctx = sizeof(*ctx);

	ms_ocall_mbedtls_net_set_block_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_set_block_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_set_block_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_set_block_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
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

	ms_ocall_mbedtls_net_set_nonblock_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_set_nonblock_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_set_nonblock_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_set_nonblock_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
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

	ms_ocall_mbedtls_net_usleep_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_usleep_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_usleep_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_usleep_t));

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

	ms_ocall_mbedtls_net_recv_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_recv_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;
	ocalloc_size += (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) ? _len_buf : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_recv_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_recv_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		ms->ms_buf = (unsigned char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_buf);
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

	ms_ocall_mbedtls_net_send_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_send_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;
	ocalloc_size += (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) ? _len_buf : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_send_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_send_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		ms->ms_buf = (unsigned char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_buf);
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

	ms_ocall_mbedtls_net_recv_timeout_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_recv_timeout_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;
	ocalloc_size += (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) ? _len_buf : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_recv_timeout_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_recv_timeout_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
		memcpy(ms->ms_ctx, ctx, _len_ctx);
	} else if (ctx == NULL) {
		ms->ms_ctx = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (buf != NULL && sgx_is_within_enclave(buf, _len_buf)) {
		ms->ms_buf = (unsigned char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_buf);
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

	ms_ocall_mbedtls_net_free_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mbedtls_net_free_t);
	void *__tmp = NULL;

	ocalloc_size += (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) ? _len_ctx : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mbedtls_net_free_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mbedtls_net_free_t));

	if (ctx != NULL && sgx_is_within_enclave(ctx, _len_ctx)) {
		ms->ms_ctx = (mbedtls_net_context*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_ctx);
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

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
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

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_cpuinfo = 4 * sizeof(*cpuinfo);

	ms_sgx_oc_cpuidex_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_oc_cpuidex_t);
	void *__tmp = NULL;

	ocalloc_size += (cpuinfo != NULL && sgx_is_within_enclave(cpuinfo, _len_cpuinfo)) ? _len_cpuinfo : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_oc_cpuidex_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_oc_cpuidex_t));

	if (cpuinfo != NULL && sgx_is_within_enclave(cpuinfo, _len_cpuinfo)) {
		ms->ms_cpuinfo = (int*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_cpuinfo);
		memcpy(ms->ms_cpuinfo, cpuinfo, _len_cpuinfo);
	} else if (cpuinfo == NULL) {
		ms->ms_cpuinfo = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_leaf = leaf;
	ms->ms_subleaf = subleaf;
	status = sgx_ocall(11, ms);

	if (cpuinfo) memcpy((void*)cpuinfo, ms->ms_cpuinfo, _len_cpuinfo);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_wait_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t));

	ms->ms_self = SGX_CAST(void*, self);
	status = sgx_ocall(12, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_set_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_untrusted_event_ocall_t));

	ms->ms_waiter = SGX_CAST(void*, waiter);
	status = sgx_ocall(13, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_setwait_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t));

	ms->ms_waiter = SGX_CAST(void*, waiter);
	ms->ms_self = SGX_CAST(void*, self);
	status = sgx_ocall(14, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_waiters = total * sizeof(*waiters);

	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);
	void *__tmp = NULL;

	ocalloc_size += (waiters != NULL && sgx_is_within_enclave(waiters, _len_waiters)) ? _len_waiters : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_multiple_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t));

	if (waiters != NULL && sgx_is_within_enclave(waiters, _len_waiters)) {
		ms->ms_waiters = (void**)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_waiters);
		memcpy((void*)ms->ms_waiters, waiters, _len_waiters);
	} else if (waiters == NULL) {
		ms->ms_waiters = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_total = total;
	status = sgx_ocall(15, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

