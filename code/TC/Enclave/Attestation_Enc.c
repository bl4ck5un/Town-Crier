#include "sgx_utils.h"
#include "sgx_report.h"
#include "string.h"
#include "time.h"
#include <ctime>
#include "keccak.h"
#include "ECDSA.h"
#include "Log.h"
#include "Enclave_t.h"
#include "Constants.h"

int ecall_create_report (sgx_target_info_t* quote_enc_info, sgx_report_t* report, time_t wall_clock, uint8_t wtc_rsv[65])
{
    long long time_tmp = 0, time_tmp2 = 0;
    sgx_report_data_t data;
    int ret = 0;
    uint8_t wtc_hash[32];
#ifdef E2E_BENCHMARK
    rdtsc(&time_tmp);
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
#ifdef E2E_BENCHMARK
    rdtsc(&time_tmp2);
    LL_CRITICAL("sign: %llu", time_tmp2 - time_tmp);
#endif
    memset( &data.d, 0x90, sizeof data.d);
    ret = sgx_create_report (quote_enc_info, &data, report);
    return ret;
}
