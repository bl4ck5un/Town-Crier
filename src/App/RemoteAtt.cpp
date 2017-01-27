#include "App.h"
#include "sgx_uae_service.h"
#include "sgx_report.h"
#include "sgx_utils.h"
#include "Enclave_u.h"

#include "RemoteAtt.h"
#include "printf.h"
#include <time.h>
#include <Log.h>
#include "Constants.h"
#include <Debug.h>
#include "utils.h"
#include "utils.h"

int time_calibrate(sgx_enclave_id_t eid)
{
    time_t wtc_time = time(NULL);
    uint8_t time_sig[65];
    int ret = 0;
    sgx_status_t st;

    st = ecall_time_calibrate(eid, &ret, wtc_time, time_sig);
#ifdef VERBOSE
    hexdump("Time", time_sig, sizeof time_sig);
#endif
    return ret;
}

int remote_att_init (sgx_enclave_id_t eid)
{
    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t report;
    sgx_spid_t spid;
    int ret;

#ifdef TIME_CALIBRATION_BENCHMARK
    long long time1, time2;
    time1 = __rdtsc();
#endif
    // get report
    sgx_init_quote( &quote_enc_info, &p_gid);
    ecall_create_report(eid, &ret, &quote_enc_info, &report);
    if (ret != SGX_SUCCESS) {
        printf("ecall_create_report returned %d", ret);
        return -1;
    }
#ifdef TIME_CALIBRATION_BENCHMARK
    time2 = __rdtsc();
    LL_CRITICAL("sgx_create_report: %f", (time2-time1)/FREQ);
#endif
    // get quote
    memset(spid.id, 0x88, sizeof spid.id);
    uint32_t quote_size;
    sgx_get_quote_size(NULL, &quote_size);
    sgx_quote_t* quote = (sgx_quote_t*) malloc(quote_size);
    ret = sgx_get_quote(&report, SGX_UNLINKABLE_SIGNATURE, &spid, NULL, NULL, 0, NULL, quote, quote_size);
    if (ret != SGX_SUCCESS)
    {
        print_error_message((sgx_status_t) ret);
        LL_CRITICAL("sgx_get_quote returned %d", ret);
        return -1;
    }
#ifdef TIME_CALIBRATION_BENCHMARK
    time1 = __rdtsc();
    LL_CRITICAL("sgx_get_quote: %f", (time1-time2)/FREQ);
#endif

    dump_buf("QUOTE: ", (uint8_t*) quote, quote_size);

#ifdef VERBOSE
    dump_buf("REPORT_MAC: ", report.mac, sizeof report.mac);
    dump_buf("REPORT_DAT: ", report.body.report_data.d, sizeof report.body.report_data.d);
    dump_buf("WTC_TIME: ", (uint8_t*) &wtc_time, sizeof wtc_time);
    dump_buf("TIME_RSV: ", time_rsv, sizeof time_rsv);
#endif

    return 0;
}
