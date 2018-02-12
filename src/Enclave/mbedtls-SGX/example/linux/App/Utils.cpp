//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TowCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include "sgx_urts.h"
#include "sgx_uae_service.h"

#include <stdio.h>
#include <string.h>

#include "Utils.h"

#ifdef _MSC_VER
# include <Shlobj.h>
#else
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX
#endif

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
        printf("Error: Unexpected error occurred.\n");
}


/* Initialize the enclave:
 *   Step 1: retrive the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave(sgx_enclave_id_t *eid)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;
    
    /* Step 1: retrive the launch token saved by last transaction */
#ifdef _MSC_VER
    /* try to get the token saved in CSIDL_LOCAL_APPDATA */
    if (S_OK != SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, token_path)) {
        strncpy_s(token_path, _countof(token_path), TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    } else {
        strncat_s(token_path, _countof(token_path), "\\" TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+2);
    }

    /* open the token file */
    HANDLE token_handler = CreateFileA(token_path, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    if (token_handler == INVALID_HANDLE_VALUE) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    } else {
        /* read the token from saved file */
        DWORD read_num = 0;
        ReadFile(token_handler, token, sizeof(sgx_launch_token_t), &read_num, NULL);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
#else /* __GNUC__ */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;
    
    if (home_dir != NULL && 
        (strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
    } else {
        /* if token path is too long or $HOME is NULL */
        strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    }

    FILE *fp = fopen(token_path, "rb");
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }

    if (fp != NULL) {
        /* read the token from saved file */
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
#endif
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, eid, NULL);
    if (ret != SGX_SUCCESS) {
        printf("sgx_create_enclave returned %#x\n", ret);
        print_error_message(ret);
#ifdef _MSC_VER
        if (token_handler != INVALID_HANDLE_VALUE)
            CloseHandle(token_handler);
#else
        if (fp != NULL) fclose(fp);
#endif
        return -1;
    }

    /* Step 3: save the launch token if it is updated */
#ifdef _MSC_VER
    if (updated == FALSE || token_handler == INVALID_HANDLE_VALUE) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (token_handler != INVALID_HANDLE_VALUE)
            CloseHandle(token_handler);
        return 0;
    }
    
    /* flush the file cache */
    FlushFileBuffers(token_handler);
    /* set access offset to the begin of the file */
    SetFilePointer(token_handler, 0, NULL, FILE_BEGIN);

    /* write back the token */
    DWORD write_num = 0;
    WriteFile(token_handler, token, sizeof(sgx_launch_token_t), &write_num, NULL);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    CloseHandle(token_handler);
#else /* __GNUC__ */
    if (updated == FALSE || fp == NULL) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != NULL) fclose(fp);
        return 0;
    }

    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;
    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    fclose(fp);
#endif
    return 0;
}

#if defined(_MSC_VER)
/* query and enable SGX device*/
int query_sgx_status()
{
    sgx_device_status_t sgx_device_status;
    sgx_status_t sgx_ret = sgx_enable_device(&sgx_device_status);
    if (sgx_ret != SGX_SUCCESS) {
        printf("Failed to get SGX device status.\n");
        return -1;
    }
    else {
        switch (sgx_device_status) {
        case SGX_ENABLED:
            return 0;
        case SGX_DISABLED_REBOOT_REQUIRED:
            printf("SGX device has been enabled. Please reboot your machine.\n");
            return -1;
        case SGX_DISABLED_LEGACY_OS:
            printf("SGX device can't be enabled on an OS that doesn't support EFI interface.\n");
            return -1;
        case SGX_DISABLED:
            printf("SGX device not found.\n");
            return -1;
        default:
            printf("Unexpected error.\n");
            return -1;
        }
    }
}
#endif
