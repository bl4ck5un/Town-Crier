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
#include "Utils.h"

//#undef E2E_BENCHMARK
int remote_att_init ()
{

    sgx_target_info_t quote_enc_info;
    sgx_epid_group_id_t p_gid;
    sgx_report_t report;
    sgx_spid_t spid;
    int ret;

    time_t wtc_time = time(NULL);
    uint8_t time_rsv[65];
    long long time1, time2;
#ifdef E2E_BENCHMARK
    time1 = __rdtsc();
#endif
    // get report
    sgx_init_quote( &quote_enc_info, &p_gid);
    ecall_create_report(global_eid, &ret, &quote_enc_info, &report, wtc_time, time_rsv);
    if (ret != SGX_SUCCESS) {
        printf("ecall_create_report returned %d", ret);
        return -1;
    }
#ifdef E2E_BENCHMARK
    time2 = __rdtsc();
    LL_CRITICAL("init + sgx_create_report: %llu", time2 - time1);
    time1 = __rdtsc();
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
#ifdef E2E_BENCHMARK
    time2 = __rdtsc();
    LL_CRITICAL("sgx_get_quote: %llu", time2 - time1);
#endif

#ifdef VERBOSE
    dump_buf("REPORT_MAC: ", report.mac, sizeof report.mac);
    dump_buf("REPORT_DAT: ", report.body.report_data.d, sizeof report.body.report_data.d);
    dump_buf("WTC_TIME: ", (uint8_t*) &wtc_time, sizeof wtc_time);
    dump_buf("TIME_RSV: ", time_rsv, sizeof time_rsv);
#endif

    return 0;
}
