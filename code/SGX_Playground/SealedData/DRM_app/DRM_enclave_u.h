#ifndef DRM_ENCLAVE_U_H__
#define DRM_ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

sgx_status_t SGX_UBRIDGE(SGX_NOCONVENTION, create_session_ocall, (uint32_t* sid, uint8_t* dh_msg1, uint32_t dh_msg1_size, uint32_t timeout));
sgx_status_t SGX_UBRIDGE(SGX_NOCONVENTION, exchange_report_ocall, (uint32_t sid, uint8_t* dh_msg2, uint32_t dh_msg2_size, uint8_t* dh_msg3, uint32_t dh_msg3_size, uint32_t timeout));
sgx_status_t SGX_UBRIDGE(SGX_NOCONVENTION, close_session_ocall, (uint32_t sid, uint32_t timeout));
sgx_status_t SGX_UBRIDGE(SGX_NOCONVENTION, invoke_service_ocall, (uint8_t* pse_message_req, uint32_t pse_message_req_size, uint8_t* pse_message_resp, uint32_t pse_message_resp_size, uint32_t timeout));
void SGX_UBRIDGE(SGX_CDECL, sgx_oc_cpuidex, (int cpuinfo[4], int leaf, int subleaf));
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_wait_untrusted_event_ocall, (const void* self));
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_untrusted_event_ocall, (const void* waiter));
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_setwait_untrusted_events_ocall, (const void* waiter, const void* self));
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_multiple_untrusted_events_ocall, (const void** waiters, size_t total));

sgx_status_t create_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size);
sgx_status_t perform_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size);
sgx_status_t update_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size);
sgx_status_t delete_sealed_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size);
sgx_status_t create_time_based_policy(sgx_enclave_id_t eid, uint32_t* retval, uint8_t* sealed_log, uint32_t sealed_log_size);
sgx_status_t perform_time_based_policy(sgx_enclave_id_t eid, uint32_t* retval, const uint8_t* sealed_log, uint32_t sealed_log_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
