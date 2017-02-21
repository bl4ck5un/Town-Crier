#include <sgx_uae_service.h>
#include <sgx_report.h>
#include <sgx_utils.h>
#include <time.h>

#include <string>

#include "Enclave_u.h"

#include "attestation.h"
#include "Log.h"
#include "Constants.h"
#include "Debug.h"
#include "utils.h"

using namespace std;

int time_calibrate(sgx_enclave_id_t eid)
{
    time_t wtc_time = time(NULL);
    uint8_t time_sig[65];
    int ret = 0;
    sgx_status_t st;

    st = ecall_time_calibrate(eid, &ret, wtc_time, time_sig);
    return ret;
}

vector<uint8_t> get_attestation(sgx_enclave_id_t eid) throw(runtime_error)
{
    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t report;
    sgx_spid_t spid;
    int ret;

    sgx_init_quote( &quote_enc_info, &p_gid);

    memset(quote_enc_info.reserved1, 0, sizeof quote_enc_info.reserved1);
    memset(quote_enc_info.reserved2, 0, sizeof quote_enc_info.reserved2);
    ecall_create_report(eid, &ret, &quote_enc_info, &report);
    if (ret != 0) {
        throw runtime_error("ecall_create_report returned " + to_string(ret));
    }

    memset(spid.id, 0x88, sizeof spid.id);
    uint32_t quote_size;
    sgx_get_quote_size(NULL, &quote_size);
    sgx_quote_t* quote = (sgx_quote_t*) malloc(quote_size);
    ret = sgx_get_quote(&report, SGX_UNLINKABLE_SIGNATURE, &spid, NULL, NULL, 0, NULL, quote, quote_size);
    if (ret != SGX_SUCCESS)
    {
        print_error_message((sgx_status_t) ret);
        throw runtime_error("sgx_get_quote returned " + to_string(ret));
    }
    vector<uint8_t > quote_binary;
    quote_binary.insert(quote_binary.begin(), (uint8_t *) quote, (uint8_t *) quote + quote_size);
    return quote_binary;
}
