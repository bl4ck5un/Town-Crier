#include "sgx_utils.h"
#include "sgx_report.h"
#include "string.h"

sgx_status_t ecall_create_report (sgx_target_info_t* quote_enc_info, sgx_report_t* report)
{
    sgx_report_data_t data;

    memset( &data.d, 0x90, sizeof data.d);
    return sgx_create_report (quote_enc_info, &data, report);
}