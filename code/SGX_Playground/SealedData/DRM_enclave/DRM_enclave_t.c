#include "DRM_enclave_t.h"

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


typedef struct ms_create_sealed_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_create_sealed_policy_t;

typedef struct ms_perform_sealed_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_perform_sealed_policy_t;

typedef struct ms_update_sealed_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_update_sealed_policy_t;

typedef struct ms_delete_sealed_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_delete_sealed_policy_t;

typedef struct ms_create_time_based_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_create_time_based_policy_t;

typedef struct ms_perform_time_based_policy_t {
	uint32_t ms_retval;
	uint8_t* ms_sealed_log;
	uint32_t ms_sealed_log_size;
} ms_perform_time_based_policy_t;

typedef struct ms_create_session_ocall_t {
	sgx_status_t ms_retval;
	uint32_t* ms_sid;
	uint8_t* ms_dh_msg1;
	uint32_t ms_dh_msg1_size;
	uint32_t ms_timeout;
} ms_create_session_ocall_t;

typedef struct ms_exchange_report_ocall_t {
	sgx_status_t ms_retval;
	uint32_t ms_sid;
	uint8_t* ms_dh_msg2;
	uint32_t ms_dh_msg2_size;
	uint8_t* ms_dh_msg3;
	uint32_t ms_dh_msg3_size;
	uint32_t ms_timeout;
} ms_exchange_report_ocall_t;

typedef struct ms_close_session_ocall_t {
	sgx_status_t ms_retval;
	uint32_t ms_sid;
	uint32_t ms_timeout;
} ms_close_session_ocall_t;

typedef struct ms_invoke_service_ocall_t {
	sgx_status_t ms_retval;
	uint8_t* ms_pse_message_req;
	uint32_t ms_pse_message_req_size;
	uint8_t* ms_pse_message_resp;
	uint32_t ms_pse_message_resp_size;
	uint32_t ms_timeout;
} ms_invoke_service_ocall_t;

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

static sgx_status_t SGX_CDECL sgx_create_sealed_policy(void* pms)
{
	ms_create_sealed_policy_t* ms = SGX_CAST(ms_create_sealed_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_create_sealed_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		if ((_in_sealed_log = (uint8_t*)malloc(_len_sealed_log)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sealed_log, 0, _len_sealed_log);
	}
	ms->ms_retval = create_sealed_policy(_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) {
		memcpy(_tmp_sealed_log, _in_sealed_log, _len_sealed_log);
		free(_in_sealed_log);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_perform_sealed_policy(void* pms)
{
	ms_perform_sealed_policy_t* ms = SGX_CAST(ms_perform_sealed_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_perform_sealed_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		_in_sealed_log = (uint8_t*)malloc(_len_sealed_log);
		if (_in_sealed_log == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_sealed_log, _tmp_sealed_log, _len_sealed_log);
	}
	ms->ms_retval = perform_sealed_policy((const uint8_t*)_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) free((void*)_in_sealed_log);

	return status;
}

static sgx_status_t SGX_CDECL sgx_update_sealed_policy(void* pms)
{
	ms_update_sealed_policy_t* ms = SGX_CAST(ms_update_sealed_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_update_sealed_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		_in_sealed_log = (uint8_t*)malloc(_len_sealed_log);
		if (_in_sealed_log == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_sealed_log, _tmp_sealed_log, _len_sealed_log);
	}
	ms->ms_retval = update_sealed_policy(_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) {
		memcpy(_tmp_sealed_log, _in_sealed_log, _len_sealed_log);
		free(_in_sealed_log);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_delete_sealed_policy(void* pms)
{
	ms_delete_sealed_policy_t* ms = SGX_CAST(ms_delete_sealed_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_delete_sealed_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		_in_sealed_log = (uint8_t*)malloc(_len_sealed_log);
		if (_in_sealed_log == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_sealed_log, _tmp_sealed_log, _len_sealed_log);
	}
	ms->ms_retval = delete_sealed_policy((const uint8_t*)_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) free((void*)_in_sealed_log);

	return status;
}

static sgx_status_t SGX_CDECL sgx_create_time_based_policy(void* pms)
{
	ms_create_time_based_policy_t* ms = SGX_CAST(ms_create_time_based_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_create_time_based_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		if ((_in_sealed_log = (uint8_t*)malloc(_len_sealed_log)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sealed_log, 0, _len_sealed_log);
	}
	ms->ms_retval = create_time_based_policy(_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) {
		memcpy(_tmp_sealed_log, _in_sealed_log, _len_sealed_log);
		free(_in_sealed_log);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_perform_time_based_policy(void* pms)
{
	ms_perform_time_based_policy_t* ms = SGX_CAST(ms_perform_time_based_policy_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_sealed_log = ms->ms_sealed_log;
	uint32_t _tmp_sealed_log_size = ms->ms_sealed_log_size;
	size_t _len_sealed_log = _tmp_sealed_log_size;
	uint8_t* _in_sealed_log = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_perform_time_based_policy_t));
	CHECK_UNIQUE_POINTER(_tmp_sealed_log, _len_sealed_log);

	if (_tmp_sealed_log != NULL) {
		_in_sealed_log = (uint8_t*)malloc(_len_sealed_log);
		if (_in_sealed_log == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy((void*)_in_sealed_log, _tmp_sealed_log, _len_sealed_log);
	}
	ms->ms_retval = perform_time_based_policy((const uint8_t*)_in_sealed_log, _tmp_sealed_log_size);
err:
	if (_in_sealed_log) free((void*)_in_sealed_log);

	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* call_addr; uint8_t is_priv;} ecall_table[6];
} g_ecall_table = {
	6,
	{
		{(void*)(uintptr_t)sgx_create_sealed_policy, 0},
		{(void*)(uintptr_t)sgx_perform_sealed_policy, 0},
		{(void*)(uintptr_t)sgx_update_sealed_policy, 0},
		{(void*)(uintptr_t)sgx_delete_sealed_policy, 0},
		{(void*)(uintptr_t)sgx_create_time_based_policy, 0},
		{(void*)(uintptr_t)sgx_perform_time_based_policy, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[9][6];
} g_dyn_entry_table = {
	9,
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
	}
};


sgx_status_t SGX_CDECL create_session_ocall(sgx_status_t* retval, uint32_t* sid, uint8_t* dh_msg1, uint32_t dh_msg1_size, uint32_t timeout)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sid = sizeof(*sid);
	size_t _len_dh_msg1 = dh_msg1_size;

	ms_create_session_ocall_t* ms;
	OCALLOC(ms, ms_create_session_ocall_t*, sizeof(*ms));

	if (sid != NULL && sgx_is_within_enclave(sid, _len_sid)) {
		OCALLOC(ms->ms_sid, uint32_t*, _len_sid);
		memset(ms->ms_sid, 0, _len_sid);
	} else if (sid == NULL) {
		ms->ms_sid = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	if (dh_msg1 != NULL && sgx_is_within_enclave(dh_msg1, _len_dh_msg1)) {
		OCALLOC(ms->ms_dh_msg1, uint8_t*, _len_dh_msg1);
		memset(ms->ms_dh_msg1, 0, _len_dh_msg1);
	} else if (dh_msg1 == NULL) {
		ms->ms_dh_msg1 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_dh_msg1_size = dh_msg1_size;
	ms->ms_timeout = timeout;
	status = sgx_ocall(0, ms);

	if (retval) *retval = ms->ms_retval;
	if (sid) memcpy((void*)sid, ms->ms_sid, _len_sid);
	if (dh_msg1) memcpy((void*)dh_msg1, ms->ms_dh_msg1, _len_dh_msg1);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL exchange_report_ocall(sgx_status_t* retval, uint32_t sid, uint8_t* dh_msg2, uint32_t dh_msg2_size, uint8_t* dh_msg3, uint32_t dh_msg3_size, uint32_t timeout)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_dh_msg2 = dh_msg2_size;
	size_t _len_dh_msg3 = dh_msg3_size;

	ms_exchange_report_ocall_t* ms;
	OCALLOC(ms, ms_exchange_report_ocall_t*, sizeof(*ms));

	ms->ms_sid = sid;
	if (dh_msg2 != NULL && sgx_is_within_enclave(dh_msg2, _len_dh_msg2)) {
		OCALLOC(ms->ms_dh_msg2, uint8_t*, _len_dh_msg2);
		memcpy(ms->ms_dh_msg2, dh_msg2, _len_dh_msg2);
	} else if (dh_msg2 == NULL) {
		ms->ms_dh_msg2 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_dh_msg2_size = dh_msg2_size;
	if (dh_msg3 != NULL && sgx_is_within_enclave(dh_msg3, _len_dh_msg3)) {
		OCALLOC(ms->ms_dh_msg3, uint8_t*, _len_dh_msg3);
		memset(ms->ms_dh_msg3, 0, _len_dh_msg3);
	} else if (dh_msg3 == NULL) {
		ms->ms_dh_msg3 = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_dh_msg3_size = dh_msg3_size;
	ms->ms_timeout = timeout;
	status = sgx_ocall(1, ms);

	if (retval) *retval = ms->ms_retval;
	if (dh_msg3) memcpy((void*)dh_msg3, ms->ms_dh_msg3, _len_dh_msg3);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL close_session_ocall(sgx_status_t* retval, uint32_t sid, uint32_t timeout)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_close_session_ocall_t* ms;
	OCALLOC(ms, ms_close_session_ocall_t*, sizeof(*ms));

	ms->ms_sid = sid;
	ms->ms_timeout = timeout;
	status = sgx_ocall(2, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL invoke_service_ocall(sgx_status_t* retval, uint8_t* pse_message_req, uint32_t pse_message_req_size, uint8_t* pse_message_resp, uint32_t pse_message_resp_size, uint32_t timeout)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pse_message_req = pse_message_req_size;
	size_t _len_pse_message_resp = pse_message_resp_size;

	ms_invoke_service_ocall_t* ms;
	OCALLOC(ms, ms_invoke_service_ocall_t*, sizeof(*ms));

	if (pse_message_req != NULL && sgx_is_within_enclave(pse_message_req, _len_pse_message_req)) {
		OCALLOC(ms->ms_pse_message_req, uint8_t*, _len_pse_message_req);
		memcpy(ms->ms_pse_message_req, pse_message_req, _len_pse_message_req);
	} else if (pse_message_req == NULL) {
		ms->ms_pse_message_req = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_pse_message_req_size = pse_message_req_size;
	if (pse_message_resp != NULL && sgx_is_within_enclave(pse_message_resp, _len_pse_message_resp)) {
		OCALLOC(ms->ms_pse_message_resp, uint8_t*, _len_pse_message_resp);
		memset(ms->ms_pse_message_resp, 0, _len_pse_message_resp);
	} else if (pse_message_resp == NULL) {
		ms->ms_pse_message_resp = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_pse_message_resp_size = pse_message_resp_size;
	ms->ms_timeout = timeout;
	status = sgx_ocall(3, ms);

	if (retval) *retval = ms->ms_retval;
	if (pse_message_resp) memcpy((void*)pse_message_resp, ms->ms_pse_message_resp, _len_pse_message_resp);

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
