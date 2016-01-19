#include "Enclave2_t.h"

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


typedef struct ms_test_create_session_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
} ms_test_create_session_t;

typedef struct ms_test_enclave_to_enclave_call_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
} ms_test_enclave_to_enclave_call_t;

typedef struct ms_test_message_exchange_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
} ms_test_message_exchange_t;

typedef struct ms_test_close_session_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
} ms_test_close_session_t;

typedef struct ms_session_request_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_dh_msg1_t* ms_dh_msg1;
	uint32_t* ms_session_id;
} ms_session_request_t;

typedef struct ms_exchange_report_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_dh_msg2_t* ms_dh_msg2;
	sgx_dh_msg3_t* ms_dh_msg3;
	uint32_t ms_session_id;
} ms_exchange_report_t;

typedef struct ms_generate_response_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	secure_message_t* ms_req_message;
	size_t ms_req_message_size;
	size_t ms_max_payload_size;
	secure_message_t* ms_resp_message;
	size_t ms_resp_message_size;
} ms_generate_response_t;

typedef struct ms_end_session_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
} ms_end_session_t;

typedef struct ms_session_request_ocall_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
	sgx_dh_msg1_t* ms_dh_msg1;
	uint32_t* ms_session_id;
} ms_session_request_ocall_t;

typedef struct ms_exchange_report_ocall_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
	sgx_dh_msg2_t* ms_dh_msg2;
	sgx_dh_msg3_t* ms_dh_msg3;
	uint32_t ms_session_id;
} ms_exchange_report_ocall_t;

typedef struct ms_send_request_ocall_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
	secure_message_t* ms_req_message;
	size_t ms_req_message_size;
	size_t ms_max_payload_size;
	secure_message_t* ms_resp_message;
	size_t ms_resp_message_size;
} ms_send_request_ocall_t;

typedef struct ms_end_session_ocall_t {
	uint32_t ms_retval;
	sgx_enclave_id_t ms_src_enclave_id;
	sgx_enclave_id_t ms_dest_enclave_id;
} ms_end_session_ocall_t;

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4200)
#endif

static sgx_status_t SGX_CDECL sgx_test_create_session(void* pms)
{
	ms_test_create_session_t* ms = SGX_CAST(ms_test_create_session_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_test_create_session_t));

	ms->ms_retval = test_create_session(ms->ms_src_enclave_id, ms->ms_dest_enclave_id);


	return status;
}

static sgx_status_t SGX_CDECL sgx_test_enclave_to_enclave_call(void* pms)
{
	ms_test_enclave_to_enclave_call_t* ms = SGX_CAST(ms_test_enclave_to_enclave_call_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_test_enclave_to_enclave_call_t));

	ms->ms_retval = test_enclave_to_enclave_call(ms->ms_src_enclave_id, ms->ms_dest_enclave_id);


	return status;
}

static sgx_status_t SGX_CDECL sgx_test_message_exchange(void* pms)
{
	ms_test_message_exchange_t* ms = SGX_CAST(ms_test_message_exchange_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_test_message_exchange_t));

	ms->ms_retval = test_message_exchange(ms->ms_src_enclave_id, ms->ms_dest_enclave_id);


	return status;
}

static sgx_status_t SGX_CDECL sgx_test_close_session(void* pms)
{
	ms_test_close_session_t* ms = SGX_CAST(ms_test_close_session_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_test_close_session_t));

	ms->ms_retval = test_close_session(ms->ms_src_enclave_id, ms->ms_dest_enclave_id);


	return status;
}

static sgx_status_t SGX_CDECL sgx_session_request(void* pms)
{
	ms_session_request_t* ms = SGX_CAST(ms_session_request_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	sgx_dh_msg1_t* _tmp_dh_msg1 = ms->ms_dh_msg1;
	size_t _len_dh_msg1 = sizeof(*_tmp_dh_msg1);
	sgx_dh_msg1_t* _in_dh_msg1 = NULL;
	uint32_t* _tmp_session_id = ms->ms_session_id;
	size_t _len_session_id = sizeof(*_tmp_session_id);
	uint32_t* _in_session_id = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_session_request_t));
	CHECK_UNIQUE_POINTER(_tmp_dh_msg1, _len_dh_msg1);
	CHECK_UNIQUE_POINTER(_tmp_session_id, _len_session_id);

	if (_tmp_dh_msg1 != NULL) {
		if ((_in_dh_msg1 = (sgx_dh_msg1_t*)malloc(_len_dh_msg1)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_dh_msg1, 0, _len_dh_msg1);
	}
	if (_tmp_session_id != NULL) {
		if ((_in_session_id = (uint32_t*)malloc(_len_session_id)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_session_id, 0, _len_session_id);
	}
	ms->ms_retval = session_request(ms->ms_src_enclave_id, _in_dh_msg1, _in_session_id);
err:
	if (_in_dh_msg1) {
		memcpy(_tmp_dh_msg1, _in_dh_msg1, _len_dh_msg1);
		free(_in_dh_msg1);
	}
	if (_in_session_id) {
		memcpy(_tmp_session_id, _in_session_id, _len_session_id);
		free(_in_session_id);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_exchange_report(void* pms)
{
	ms_exchange_report_t* ms = SGX_CAST(ms_exchange_report_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	sgx_dh_msg2_t* _tmp_dh_msg2 = ms->ms_dh_msg2;
	size_t _len_dh_msg2 = sizeof(*_tmp_dh_msg2);
	sgx_dh_msg2_t* _in_dh_msg2 = NULL;
	sgx_dh_msg3_t* _tmp_dh_msg3 = ms->ms_dh_msg3;
	size_t _len_dh_msg3 = sizeof(*_tmp_dh_msg3);
	sgx_dh_msg3_t* _in_dh_msg3 = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_exchange_report_t));
	CHECK_UNIQUE_POINTER(_tmp_dh_msg2, _len_dh_msg2);
	CHECK_UNIQUE_POINTER(_tmp_dh_msg3, _len_dh_msg3);

	if (_tmp_dh_msg2 != NULL) {
		_in_dh_msg2 = (sgx_dh_msg2_t*)malloc(_len_dh_msg2);
		if (_in_dh_msg2 == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_dh_msg2, _tmp_dh_msg2, _len_dh_msg2);
	}
	if (_tmp_dh_msg3 != NULL) {
		if ((_in_dh_msg3 = (sgx_dh_msg3_t*)malloc(_len_dh_msg3)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_dh_msg3, 0, _len_dh_msg3);
	}
	ms->ms_retval = exchange_report(ms->ms_src_enclave_id, _in_dh_msg2, _in_dh_msg3, ms->ms_session_id);
err:
	if (_in_dh_msg2) free(_in_dh_msg2);
	if (_in_dh_msg3) {
		memcpy(_tmp_dh_msg3, _in_dh_msg3, _len_dh_msg3);
		free(_in_dh_msg3);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_generate_response(void* pms)
{
	ms_generate_response_t* ms = SGX_CAST(ms_generate_response_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	secure_message_t* _tmp_req_message = ms->ms_req_message;
	size_t _tmp_req_message_size = ms->ms_req_message_size;
	size_t _len_req_message = _tmp_req_message_size;
	secure_message_t* _in_req_message = NULL;
	secure_message_t* _tmp_resp_message = ms->ms_resp_message;
	size_t _tmp_resp_message_size = ms->ms_resp_message_size;
	size_t _len_resp_message = _tmp_resp_message_size;
	secure_message_t* _in_resp_message = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_generate_response_t));
	CHECK_UNIQUE_POINTER(_tmp_req_message, _len_req_message);
	CHECK_UNIQUE_POINTER(_tmp_resp_message, _len_resp_message);

	if (_tmp_req_message != NULL) {
		_in_req_message = (secure_message_t*)malloc(_len_req_message);
		if (_in_req_message == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_req_message, _tmp_req_message, _len_req_message);
	}
	if (_tmp_resp_message != NULL) {
		if ((_in_resp_message = (secure_message_t*)malloc(_len_resp_message)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_resp_message, 0, _len_resp_message);
	}
	ms->ms_retval = generate_response(ms->ms_src_enclave_id, _in_req_message, _tmp_req_message_size, ms->ms_max_payload_size, _in_resp_message, _tmp_resp_message_size);
err:
	if (_in_req_message) free(_in_req_message);
	if (_in_resp_message) {
		memcpy(_tmp_resp_message, _in_resp_message, _len_resp_message);
		free(_in_resp_message);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_end_session(void* pms)
{
	ms_end_session_t* ms = SGX_CAST(ms_end_session_t*, pms);
	sgx_status_t status = SGX_SUCCESS;

	CHECK_REF_POINTER(pms, sizeof(ms_end_session_t));

	ms->ms_retval = end_session(ms->ms_src_enclave_id);


	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* call_addr; uint8_t is_priv;} ecall_table[8];
} g_ecall_table = {
	8,
	{
		{(void*)(uintptr_t)sgx_test_create_session, 0},
		{(void*)(uintptr_t)sgx_test_enclave_to_enclave_call, 0},
		{(void*)(uintptr_t)sgx_test_message_exchange, 0},
		{(void*)(uintptr_t)sgx_test_close_session, 0},
		{(void*)(uintptr_t)sgx_session_request, 0},
		{(void*)(uintptr_t)sgx_exchange_report, 0},
		{(void*)(uintptr_t)sgx_generate_response, 0},
		{(void*)(uintptr_t)sgx_end_session, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[9][8];
} g_dyn_entry_table = {
	9,
	{
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL session_request_ocall(uint32_t* retval, sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, sgx_dh_msg1_t* dh_msg1, uint32_t* session_id)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_dh_msg1 = sizeof(*dh_msg1);
	size_t _len_session_id = sizeof(*session_id);

	ms_session_request_ocall_t* ms;
	OCALLOC(ms, ms_session_request_ocall_t*, sizeof(*ms));

	ms->ms_src_enclave_id = src_enclave_id;
	ms->ms_dest_enclave_id = dest_enclave_id;
	if (dh_msg1 != NULL && sgx_is_within_enclave(dh_msg1, _len_dh_msg1)) {
		OCALLOC(ms->ms_dh_msg1, sgx_dh_msg1_t*, _len_dh_msg1);
		memset(ms->ms_dh_msg1, 0, _len_dh_msg1);
	} else if (dh_msg1 == NULL) {
		ms->ms_dh_msg1 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (session_id != NULL && sgx_is_within_enclave(session_id, _len_session_id)) {
		OCALLOC(ms->ms_session_id, uint32_t*, _len_session_id);
		memset(ms->ms_session_id, 0, _len_session_id);
	} else if (session_id == NULL) {
		ms->ms_session_id = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(0, ms);

	if (retval) *retval = ms->ms_retval;
	if (dh_msg1) memcpy((void*)dh_msg1, ms->ms_dh_msg1, _len_dh_msg1);
	if (session_id) memcpy((void*)session_id, ms->ms_session_id, _len_session_id);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL exchange_report_ocall(uint32_t* retval, sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, sgx_dh_msg2_t* dh_msg2, sgx_dh_msg3_t* dh_msg3, uint32_t session_id)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_dh_msg2 = sizeof(*dh_msg2);
	size_t _len_dh_msg3 = sizeof(*dh_msg3);

	ms_exchange_report_ocall_t* ms;
	OCALLOC(ms, ms_exchange_report_ocall_t*, sizeof(*ms));

	ms->ms_src_enclave_id = src_enclave_id;
	ms->ms_dest_enclave_id = dest_enclave_id;
	if (dh_msg2 != NULL && sgx_is_within_enclave(dh_msg2, _len_dh_msg2)) {
		OCALLOC(ms->ms_dh_msg2, sgx_dh_msg2_t*, _len_dh_msg2);
		memcpy(ms->ms_dh_msg2, dh_msg2, _len_dh_msg2);
	} else if (dh_msg2 == NULL) {
		ms->ms_dh_msg2 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (dh_msg3 != NULL && sgx_is_within_enclave(dh_msg3, _len_dh_msg3)) {
		OCALLOC(ms->ms_dh_msg3, sgx_dh_msg3_t*, _len_dh_msg3);
		memset(ms->ms_dh_msg3, 0, _len_dh_msg3);
	} else if (dh_msg3 == NULL) {
		ms->ms_dh_msg3 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_session_id = session_id;
	status = sgx_ocall(1, ms);

	if (retval) *retval = ms->ms_retval;
	if (dh_msg3) memcpy((void*)dh_msg3, ms->ms_dh_msg3, _len_dh_msg3);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL send_request_ocall(uint32_t* retval, sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, secure_message_t* req_message, size_t req_message_size, size_t max_payload_size, secure_message_t* resp_message, size_t resp_message_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_req_message = req_message_size;
	size_t _len_resp_message = resp_message_size;

	ms_send_request_ocall_t* ms;
	OCALLOC(ms, ms_send_request_ocall_t*, sizeof(*ms));

	ms->ms_src_enclave_id = src_enclave_id;
	ms->ms_dest_enclave_id = dest_enclave_id;
	if (req_message != NULL && sgx_is_within_enclave(req_message, _len_req_message)) {
		OCALLOC(ms->ms_req_message, secure_message_t*, _len_req_message);
		memcpy(ms->ms_req_message, req_message, _len_req_message);
	} else if (req_message == NULL) {
		ms->ms_req_message = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_req_message_size = req_message_size;
	ms->ms_max_payload_size = max_payload_size;
	if (resp_message != NULL && sgx_is_within_enclave(resp_message, _len_resp_message)) {
		OCALLOC(ms->ms_resp_message, secure_message_t*, _len_resp_message);
		memset(ms->ms_resp_message, 0, _len_resp_message);
	} else if (resp_message == NULL) {
		ms->ms_resp_message = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_resp_message_size = resp_message_size;
	status = sgx_ocall(2, ms);

	if (retval) *retval = ms->ms_retval;
	if (resp_message) memcpy((void*)resp_message, ms->ms_resp_message, _len_resp_message);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL end_session_ocall(uint32_t* retval, sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_end_session_ocall_t* ms;
	OCALLOC(ms, ms_end_session_ocall_t*, sizeof(*ms));

	ms->ms_src_enclave_id = src_enclave_id;
	ms->ms_dest_enclave_id = dest_enclave_id;
	status = sgx_ocall(3, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_cpuinfo = 4 * sizeof(*cpuinfo);

	ms_sgx_oc_cpuidex_t* ms;
	OCALLOC(ms, ms_sgx_oc_cpuidex_t*, sizeof(*ms));

	if (cpuinfo != NULL && sgx_is_within_enclave(cpuinfo, _len_cpuinfo)) {
		OCALLOC(ms->ms_cpuinfo, int*, _len_cpuinfo);
		memcpy(ms->ms_cpuinfo, cpuinfo, _len_cpuinfo);
	} else if (cpuinfo == NULL) {
		ms->ms_cpuinfo = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_leaf = leaf;
	ms->ms_subleaf = subleaf;
	status = sgx_ocall(4, ms);

	if (cpuinfo) memcpy((void*)cpuinfo, ms->ms_cpuinfo, _len_cpuinfo);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_wait_untrusted_event_ocall_t* ms;
	OCALLOC(ms, ms_sgx_thread_wait_untrusted_event_ocall_t*, sizeof(*ms));

	ms->ms_self = SGX_CAST(void*, self);
	status = sgx_ocall(5, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_set_untrusted_event_ocall_t* ms;
	OCALLOC(ms, ms_sgx_thread_set_untrusted_event_ocall_t*, sizeof(*ms));

	ms->ms_waiter = SGX_CAST(void*, waiter);
	status = sgx_ocall(6, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms;
	OCALLOC(ms, ms_sgx_thread_setwait_untrusted_events_ocall_t*, sizeof(*ms));

	ms->ms_waiter = SGX_CAST(void*, waiter);
	ms->ms_self = SGX_CAST(void*, self);
	status = sgx_ocall(7, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_waiters = total * sizeof(*waiters);

	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms;
	OCALLOC(ms, ms_sgx_thread_set_multiple_untrusted_events_ocall_t*, sizeof(*ms));

	if (waiters != NULL && sgx_is_within_enclave(waiters, _len_waiters)) {
		OCALLOC(ms->ms_waiters, void**, _len_waiters);
		memcpy((void*)ms->ms_waiters, waiters, _len_waiters);
	} else if (waiters == NULL) {
		ms->ms_waiters = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_total = total;
	status = sgx_ocall(8, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
