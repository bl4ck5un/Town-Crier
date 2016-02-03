#include "seal.h"
#include "Enclave.h"
#include "sgx_tseal.h"

int create_nonce(int init, uint8_t* sealed, int sealed_len)
{
    mbedtls_mpi p;
    uint8_t tmp[32] = {0};
    sgx_status_t st;
    int ret;

    if (sealed_len != sgx_calc_sealed_data_size(0, 32))
    {
        printf("Invalid parameter.\n"); 
        ret = -1; goto cleanup;
    }

    mbedtls_mpi_init(&p);
    MBEDTLS_MPI_CHK (mbedtls_mpi_add_int(&p, &p, init));
    MBEDTLS_MPI_CHK (mbedtls_mpi_write_binary(&p, tmp, 32));

    st = sgx_seal_data(0, NULL, 
                       32, tmp, 
                       sealed_len, 
                       (sgx_sealed_data_t*) sealed);

    if (st != SGX_SUCCESS)
    {
        printf("Error in sealing nonce!\n");
        ret = -1; goto cleanup;
    }

cleanup:
    return ret;
}


int inc_nonce_by_one(uint8_t* sealed_nonce, int sealed_len)
{
    sgx_status_t st;
    int ret;
    size_t nonce_len;
    nonce_t nonce;
    mbedtls_mpi p;
    mbedtls_mpi_init(&p);

    if (sealed_nonce)
    {
        nonce_len = sgx_get_encrypt_txt_len((sgx_sealed_data_t*) sealed_nonce);
        if (nonce_len > 32) {printf("Error: nonce length is not right!\n"); return -1;}
        st = sgx_unseal_data ((sgx_sealed_data_t*)sealed_nonce, NULL, NULL, nonce.b, &nonce_len);
        if (nonce_len != 32) {printf("Interesting! nonce_len returned %d\n", nonce_len);}
        if (st != SGX_SUCCESS)
        {
            printf("sgx_unseal_data returned %d\n", st);
            return -1;
        }
        
        MBEDTLS_MPI_CHK (mbedtls_mpi_read_binary(&p, nonce.b, 32));
        MBEDTLS_MPI_CHK (mbedtls_mpi_add_int(&p, &p, 1));
        MBEDTLS_MPI_CHK (mbedtls_mpi_write_binary(&p, nonce.b, 32));

        st = sgx_seal_data(0, NULL, 32, nonce.b, sealed_len, (sgx_sealed_data_t*)sealed_nonce);

        if (st != SGX_SUCCESS)
        {
            printf("Error in sealing data %#x\n", st);
            ret = -1; goto cleanup;
        }
        return 0;
    }
    
    printf("NULL input\n!"); return -1;
cleanup:
    mbedtls_mpi_free(&p);
    return ret;
}
