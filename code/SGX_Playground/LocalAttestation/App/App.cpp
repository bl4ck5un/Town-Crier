/**
*   Copyright(C) 2011-2015 Intel Corporation All Rights Reserved.
*
*   The source code, information  and  material ("Material") contained herein is
*   owned  by Intel Corporation or its suppliers or licensors, and title to such
*   Material remains  with Intel Corporation  or its suppliers or licensors. The
*   Material  contains proprietary information  of  Intel or  its  suppliers and
*   licensors. The  Material is protected by worldwide copyright laws and treaty
*   provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way  without Intel's  prior  express written  permission. No  license
*   under  any patent, copyright  or  other intellectual property rights  in the
*   Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
*   implication, inducement,  estoppel or  otherwise.  Any  license  under  such
*   intellectual  property  rights must  be express  and  approved  by  Intel in
*   writing.
*
*   *Third Party trademarks are the property of their respective owners.
*
*   Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
*   this  notice or  any other notice embedded  in Materials by Intel or Intel's
*   suppliers or licensors in any way.
*/

// App.cpp : Defines the entry point for the console application.
#include <stdio.h>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#include <map>
#include "../Enclave1/Enclave1_u.h"
#include "../Enclave2/Enclave2_u.h"
#include "../Enclave3/Enclave3_u.h"
#include "sgx_eid.h"
#include "sgx_urts.h"

#define UNUSED(val) (void)(val)
#ifndef _MSC_VER
#define TCHAR   char
#define _TCHAR  char
#define _T(str) str
#define scanf_s scanf
#define _tmain  main
#endif

extern std::map<sgx_enclave_id_t, uint32_t>g_enclave_id_map;


sgx_enclave_id_t e1_enclave_id = 0;
sgx_enclave_id_t e2_enclave_id = 0;
sgx_enclave_id_t e3_enclave_id = 0;

#ifdef _MSC_VER
#define ENCLAVE1_PATH "Enclave1.signed.dll"
#define ENCLAVE2_PATH "Enclave2.signed.dll"
#define ENCLAVE3_PATH "Enclave3.signed.dll"
#else
#define ENCLAVE1_PATH "libenclave1.so"
#define ENCLAVE2_PATH "libenclave2.so"
#define ENCLAVE3_PATH "libenclave3.so"
#endif

void waitForKeyPress()
{
    uint8_t    ch;
    printf("\n\nHit a key....\n");
    scanf_s("%c", &ch);
}

uint32_t load_enclaves()
{
    uint32_t enclave_temp_no;
    int ret, launch_token_updated;
    sgx_launch_token_t launch_token;

    enclave_temp_no = 0;

    ret = sgx_create_enclave(ENCLAVE1_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e1_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e1_enclave_id, enclave_temp_no));

    ret = sgx_create_enclave(ENCLAVE2_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e2_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e2_enclave_id, enclave_temp_no));

    ret = sgx_create_enclave(ENCLAVE3_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e3_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e3_enclave_id, enclave_temp_no));



    return SGX_SUCCESS;
}

int _tmain(int argc, _TCHAR* argv[])
{
    uint32_t ret_status;
    sgx_status_t status;

    UNUSED(argc);
    UNUSED(argv);

    if(load_enclaves() != SGX_SUCCESS)
    {
        printf("\nLoad Enclave Failure");
    }

    printf("\nAvaliable Enclaves");
    printf("\nEnclave1 - EnclaveID %llx",e1_enclave_id);
    printf("\nEnclave2 - EnclaveID %llx",e2_enclave_id);
    printf("\nEnclave3 - EnclaveID %llx",e3_enclave_id);
    
    do
    {
        //Test Create session between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\nSession establishment and key exchange failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }

        //Test Enclave to Enclave call between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_enclave_to_enclave_call(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_message_exchange(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test Create session between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_enclave_to_enclave_call(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_message_exchange(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }

        //Test Create session between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_create_session(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_enclave_to_enclave_call(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_message_exchange(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
    
        //Test Create session between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_create_session(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_create_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nSecure Channel Establishment between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nSession establishment and key exchange failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }
        //Test Enclave to Enclave call between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_enclave_to_enclave_call(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_enclave_to_enclave_call Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nEnclave to Enclave Call between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nEnclave to Enclave Call failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }
        //Test message exchange between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_message_exchange(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_message_exchange Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nMessage Exchange between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nMessage Exchange failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }


        //Test Closing Session between Enclave1(Source) and Enclave2(Destination)
        status = Enclave1_test_close_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E1) and Destination (E2) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E1) and Destination (E2): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave1(Source) and Enclave3(Destination)
        status = Enclave1_test_close_session(e1_enclave_id, &ret_status, e1_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave1_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E1) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E1) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave2(Source) and Enclave3(Destination)
        status = Enclave2_test_close_session(e2_enclave_id, &ret_status, e2_enclave_id, e3_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave2_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E2) and Destination (E3) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E2) and Destination (E3): Error code is %x", ret_status);
                break;
            }
        }
        //Test Closing Session between Enclave3(Source) and Enclave1(Destination)
        status = Enclave3_test_close_session(e3_enclave_id, &ret_status, e3_enclave_id, e1_enclave_id);
        if (status!=SGX_SUCCESS)
        {
            printf("Enclave3_test_close_session Ecall failed: Error code is %x", status);
            break;
        }
        else
        {
            if(ret_status==0)
            {
                printf("\n\nClose Session between Source (E3) and Destination (E1) Enclaves successful !!!");
            }
            else
            {
                printf("\n\nClose session failure between Source (E3) and Destination (E1): Error code is %x", ret_status);
                break;
            }
        }

#pragma warning (push)
#pragma warning (disable : 4127)    
    }while(0);
#pragma warning (pop)

    sgx_destroy_enclave(e1_enclave_id);
    sgx_destroy_enclave(e2_enclave_id);
    sgx_destroy_enclave(e3_enclave_id);

    waitForKeyPress();

    return 0;
}
