#!/bin/sh
#
# Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
#
# Permission to use the "TownCrier" software ("TownCrier"), officially
# docketed at the Center for Technology Licensing at Cornell University
# as D-7364, developed through research conducted at Cornell University,
# and its associated copyrights solely for educational, research and
# non-profit purposes without fee is hereby granted, provided that the
# user agrees as follows:
#
# The permission granted herein is solely for the purpose of compiling
# the TowCrier source code. No other rights to use TownCrier and its
# associated copyrights for any other purpose are granted herein,
# whether commercial or non-commercial.
#
# Those desiring to incorporate TownCrier software into commercial
# products or use TownCrier and its associated copyrights for commercial
# purposes must contact the Center for Technology Licensing at Cornell
# University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
# ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
# commercial license.
#
# IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
# INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
# ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
# UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
# ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
# REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
# OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
# OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
# PATENT, TRADEMARK OR OTHER RIGHTS.
#
# TownCrier was developed with funding in part by the National Science
# Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
# CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
# Google Faculty Research Awards, and a VMWare Research Award.
#
# Measure heap usage (and performance) of ECC operations with various values of
# the relevant tunable compile-time parameters.
#
# Usage (preferably on a 32-bit platform):
# cmake -D CMAKE_BUILD_TYPE=Release .
# scripts/ecc-heap.sh | tee ecc-heap.log

set -eu

CONFIG_H='include/mbedtls/config.h'

if [ -r $CONFIG_H ]; then :; else
    echo "$CONFIG_H not found" >&2
    exit 1
fi

if grep -i cmake Makefile >/dev/null; then :; else
    echo "Needs Cmake" >&2
    exit 1
fi

if git status | grep -F $CONFIG_H >/dev/null 2>&1; then
    echo "config.h not clean" >&2
    exit 1
fi

CONFIG_BAK=${CONFIG_H}.bak
cp $CONFIG_H $CONFIG_BAK

cat << EOF >$CONFIG_H
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_MEMORY_DEBUG

#define MBEDTLS_TIMING_C

#define MBEDTLS_BIGNUM_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDH_C

#define MBEDTLS_ECP_DP_SECP192R1_ENABLED
#define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED

#include "check_config.h"

//#define MBEDTLS_ECP_WINDOW_SIZE            6
//#define MBEDTLS_ECP_FIXED_POINT_OPTIM      1
EOF

for F in 0 1; do
    for W in 2 3 4 5 6; do
        scripts/config.pl set MBEDTLS_ECP_WINDOW_SIZE $W
        scripts/config.pl set MBEDTLS_ECP_FIXED_POINT_OPTIM $F
        make benchmark >/dev/null 2>&1
        echo "fixed point optim = $F, max window size = $W"
        echo "--------------------------------------------"
        programs/test/benchmark
    done
done

# cleanup

mv $CONFIG_BAK $CONFIG_H
make clean
