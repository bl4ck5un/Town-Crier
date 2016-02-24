#ifndef APP_REMOTE_ATT_H
#define APP_REMOET_ATT_H

#if defined(__cplusplus)
extern "C" {
#endif


int remote_att_init (sgx_enclave_id_t eid);
int time_calibrate(sgx_enclave_id_t eid);

#if defined(__cplusplus)
}
#endif

#endif