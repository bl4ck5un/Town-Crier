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

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_


#pragma once

typedef uint32_t ATTESTATION_STATUS;


#define SUCCESS                            0x00
#define INVALID_PARAMETER                0xE1
#define VALID_SESSION                    0xE2
#define INVALID_SESSION                    0xE3
#define ATTESTATION_ERROR                0xE4
#define ATTESTATION_SE_ERROR            0xE5
#define IPP_ERROR                        0xE6
#define NO_AVAILABLE_SESSION_ERROR        0xE7
#define MALLOC_ERROR                    0xE8
#define ERROR_TAG_MISMATCH                0xE9
#define OUT_BUFFER_LENGTH_ERROR            0xEA
#define INVALID_REQUEST_TYPE_ERROR        0xEB
#define INVALID_PARAMETER_ERROR            0xEC
#define ENCLAVE_TRUST_ERROR                0xED
#define ENCRYPT_DECRYPT_ERROR            0xEE
#define DUPLICATE_SESSION                0xEF
#endif