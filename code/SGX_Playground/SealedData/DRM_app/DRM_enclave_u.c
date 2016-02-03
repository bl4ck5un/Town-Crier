#include "DRM_enclave_u.h"

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

static sgx_status_t SGX_CDECL DRM_enclave_create_session_ocall(void* pms)
{
	ms_create_session_ocall_t* ms = SGX_CAST(ms_create_session_ocall_t*, pms);
	ms->ms_retval = create_session_ocall(ms->ms_sid, ms->ms_dh_msg1, ms->ms_dh_msg1_size, ms->ms_timeout);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_exchange_report_ocall(void* pms)
{
	ms_exchange_report_ocall_t* ms = SGX_CAST(ms_exchange_report_ocall_t*, pms);
	ms->ms_retval = exchange_report_ocall(ms->ms_sid, ms->ms_dh_msg2, ms->ms_dh_msg2_size, ms->ms_dh_msg3, ms->ms_dh_msg3_size, ms->ms_timeout);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_close_session_ocall(void* pms)
{
	ms_close_session_ocall_t* ms = SGX_CAST(ms_close_session_ocall_t*, pms);
	ms->ms_retval = close_session_ocall(ms->ms_sid, ms->ms_timeout);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_invoke_service_ocall(void* pms)
{
	ms_invoke_service_ocall_t* ms = SGX_CAST(ms_invoke_service_ocall_t*, pms);
	ms->ms_retval = invoke_service_ocall(ms->ms_pse_message_req, ms->ms_pse_message_req_size, ms->ms_pse_message_resp, ms->ms_pse_message_resp_size, ms->ms_timeout);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_sgx_oc_cpuidex(void* pms)
{
	ms_sgx_oc_cpuidex_t* ms = SGX_CAST(ms_sgx_oc_cpuidex_t*, pms);
	sgx_oc_cpuidex(ms->ms_cpuinfo, ms->ms_leaf, ms->ms_subleaf);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_sgx_thread_wait_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_wait_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_wait_untrusted_event_ocall((const void*)ms->ms_self);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_sgx_thread_set_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_set_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_untrusted_event_ocall((const void*)ms->ms_waiter);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_sgx_thread_setwait_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_setwait_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_setwait_untrusted_events_ocall((const void*)ms->ms_waiter, (const void*)ms->ms_self);
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL DRM_enclave_sgx_thread_set_multiple_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_multiple_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_multiple_untrusted_events_ocall((const void**)ms->ms_waiters, ms->ms_total);
	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * func_addr[9];
} ocall_table_DRM_enclave = {
	9,
	{
		(void*)(uintptr_t)DRM_enclave_create_session_ocall,
		(void*)(uintptr_t)DRM_enclave_exchange_report_ocall,
		(void*)(uintptr_t)DRM_enclave_close_session_ocall,
		(void*)(uintptr_t)DRM_enclave_invoke_service_ocall,
		(void*)(uintptr_t)DRM_enclave_sgx_oc_cpuidex,
		(void*)(uintptr_t)DRM_enclave_sgx_thread_wait_untrusted_event_ocall,
		(void*)(uintptr_t)DRM_enclave_sgx_thread_set_untrusted_event_ocall,
		(void*)(uintptr_t)DRM_enclave_sgx_thread_setwait_untrusted_events_ocall,
		(void*)(uintptr_t)DRM_enclave_sgx_thread_set_multiple_untrusted_events_ocall,
	}
};

sgx_status_t create_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_create_sealed_policy_t ms;
	ms.ms_sealed_log = sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 0, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t perform_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_perform_sealed_policy_t ms;
	ms.ms_sealed_log = (uint8_t*)sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 1, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t update_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_update_sealed_policy_t ms;
	ms.ms_sealed_log = sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 2, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t delete_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_delete_sealed_policy_t ms;
	ms.ms_sealed_log = (uint8_t*)sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 3, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t create_time_based_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_create_time_based_policy_t ms;
	ms.ms_sealed_log = sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 4, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t perform_time_based_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size)
{
	sgx_status_t status;
	ms_perform_time_based_policy_t ms;
	ms.ms_sealed_log = (uint8_t*)sealed_log;
	ms.ms_sealed_log_size = sealed_log_size;
	status = sgx_ecall(eid, 5, &ocall_table_DRM_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

