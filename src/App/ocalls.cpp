#include <iostream>
#include <string>
#include <ctime>
#include "Enclave_u.h"
#include <sgx_tseal.h>

#ifdef _WIN32
#include "windows.h"
#endif

#include <Log.h>
#include <unistd.h>

int ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    int ret = printf("%s", str);
    fflush(stdout);
    return ret;
}

/* defining log functions for enclave's usage */

#define __OCALL_LOG_FUNC(LVL) \
void ocall_log_##LVL(const char* str) {\
  LOG_F(LVL, "%s", str);\
}

__OCALL_LOG_FUNC(FATAL)
__OCALL_LOG_FUNC(ERROR)
__OCALL_LOG_FUNC(WARNING)
__OCALL_LOG_FUNC(INFO)
__OCALL_LOG_FUNC(1)
__OCALL_LOG_FUNC(2)
__OCALL_LOG_FUNC(3)
__OCALL_LOG_FUNC(4)
__OCALL_LOG_FUNC(5)
__OCALL_LOG_FUNC(6)
__OCALL_LOG_FUNC(7)
__OCALL_LOG_FUNC(8)
__OCALL_LOG_FUNC(9)

long long rdtsc()
{
#ifdef _WIN32
    return __rdtsc();
#else
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
#endif
}

void ocall_sleep(int milisec)
{
    LL_INFO("Waiting for %d", milisec/1000);
    for (int i = 0; i < milisec / 1000; i++)
    {
        printf(".");
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }
    printf("\n");
}

time_t ocall_time()
{
    return time(NULL);
}

void write(uint32_t sealed_data_size, sgx_sealed_data_t * p_sealed_data, char* filename)
{
	FILE* fp = fopen(filename, "wb");
	fwrite(p_sealed_data, sealed_data_size, 1, fp);
}
