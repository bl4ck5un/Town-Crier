#include "App.h"
#include "sgx_uae_service.h"
#include "sgx_report.h"
#include "sgx_utils.h"
#include "Enclave_u.h"

#include "RemoteAtt.h"
#include "printf.h"

int remote_att_init ()
{
    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_status_t ret;
    sgx_report_t report;


    sgx_init_quote( &quote_enc_info, &p_gid);
    ecall_create_report(global_eid, &ret, &quote_enc_info, &report);

    if (ret != SGX_SUCCESS) {
        printf("ecall_create_report returned %d", ret);
        return -1;
    }

    dump_buf("REPORT_MAC: ", report.mac, sizeof report.mac);
    dump_buf("REPORT_DAT: ", report.body.report_data.d, sizeof report.body.report_data.d);

    return 0;
}