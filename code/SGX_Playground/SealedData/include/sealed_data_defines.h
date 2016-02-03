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


#pragma once
#ifndef _SEALED_DATA_DEFINES_H_
#define _SEALED_DATA_DEFINES_H_

#include "sgx_error.h"

#define PLATFORM_SERVICE_DOWNGRADED 0xF001

#define REPLAY_DETECTED             0xF002
#define MAX_RELEASE_REACHED         0xF003

/* equal to sgx_calc_sealed_data_size(0,sizeof(replay_protected_pay_load))) */ 
#define SEALED_REPLAY_PROTECTED_PAY_LOAD_SIZE 620
#define REPLAY_PROTECTED_PAY_LOAD_MAX_RELEASE_VERSION 5

#define TIMESOURCE_CHANGED          0xF004
#define TIMESTAMP_UNEXPECTED        0xF005
#define LEASE_EXPIRED               0xF006

/* equal tosgx_calc_sealed_data_size(0,sizeof(time_based_pay_load))) */ 
#define TIME_BASED_PAY_LOAD_SIZE 624
#define TIME_BASED_LEASE_DURATION_SECOND 3

#endif
