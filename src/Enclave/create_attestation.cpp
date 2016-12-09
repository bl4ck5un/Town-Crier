#include "sgx_utils.h"
#include "sgx_report.h"
#include "string.h"
#include "time.h"
#include "external/keccak.h"
#include "eth_ecdsa.h"
#include "Log.h"
#include "Enclave_t.h"
#include "Constants.h"

int ecall_create_report (sgx_target_info_t* quote_enc_info, sgx_report_t* report)
{
    sgx_report_data_t data; // user defined data
    int ret = 0;
    memset( &data.d, 0x90, sizeof data.d); // put in some garbage
    ret = sgx_create_report (quote_enc_info, &data, report);
    return ret;
}

int ecall_time_calibrate (time_t wall_clock, uint8_t wtc_rsv[65])
{
    int ret = 0;
    uint8_t wtc_hash[32];
#ifdef TIME_CALIBRATION_BENCHMARK
    long long time1 = 0, time2 = 0;
    rdtsc(&time1);
#endif
    ret = keccak((uint8_t*)&wall_clock, sizeof wall_clock, wtc_hash, 32);
    if (ret != 0)
    {
        LL_CRITICAL("keccak returned %d", ret);
        return ret;
    }
    ret = sign(wtc_hash, sizeof wtc_hash, wtc_rsv, wtc_rsv + 32, wtc_rsv + 64);
    if (ret != 0)
    {
        LL_CRITICAL("sign() returned %d", ret);
        return ret;
    }
#ifdef TIME_CALIBRATION_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("sign the timestamp: %f", (time2-time1)/FREQ);
#endif
    return ret;
}
