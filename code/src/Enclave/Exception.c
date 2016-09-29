#include "sgx_trts_exception.h"
#include "Log.h"

int UD_handler (sgx_exception_info_t *e)
{
    LL_CRITICAL("%d exception occurs", e->exception_vector);
    return 0;
}

int register_exception_handlers(){
    void* p = sgx_register_exception_handler(1, UD_handler);
    if (p == NULL)
        return -1;
    else
        return 0;
}
