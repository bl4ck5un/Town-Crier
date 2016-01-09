// app_using_enclave.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <tchar.h>
#include "sgx_urts.h"
#include "simple_enclave_u.h"
 
#define ENCLAVE_FILE _T("simple_enclave.signed.dll")
#define MAX_BUF_LEN 100
 
int main()
{
	sgx_enclave_id_t   eid;
	sgx_status_t       ret   = SGX_SUCCESS;
	sgx_launch_token_t token = {0};
	int updated = 0;
	char buffer[MAX_BUF_LEN] = "Hello World!";
 
	// Create the Enclave with above launch token.
	ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated,
							 &eid, NULL);
	if (ret != SGX_SUCCESS) {
		printf("App: error %#x, failed to create enclave.\n", ret);
		goto error;
	}
 
	// A bunch of Enclave calls (ECALL) will happen here.
	load_from_enc(eid, buffer, MAX_BUF_LEN);
	printf("%s", buffer);
 
	// Destroy the enclave when all Enclave calls finished.
	if(SGX_SUCCESS != sgx_destroy_enclave(eid))
		goto error;
 
	getchar();
	return 0;

error:
	getchar();
	return -1;
}

