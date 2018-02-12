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
set -eu

if [ $# -ne 2 ]; then
    echo "Usage: $0 path/to/config.pl path/to/config.h" >&2
    exit 1
fi

SCRIPT=$1
FILE=$2

conf() {
    $SCRIPT -f $FILE $@
}

# not supported on mbed OS, nor used by mbed Client
conf unset MBEDTLS_NET_C
conf unset MBEDTLS_TIMING_C

# not supported on all targets with mbed OS, nor used by mbed Client
conf unset MBEDTLS_FS_IO

conf unset MBEDTLS_CIPHER_MODE_CFB
conf unset MBEDTLS_CIPHER_MODE_CTR
conf unset MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
conf unset MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
conf unset MBEDTLS_CIPHER_PADDING_ZEROS
conf unset MBEDTLS_ECP_DP_SECP192R1_ENABLED
conf unset MBEDTLS_ECP_DP_SECP224R1_ENABLED
conf unset MBEDTLS_ECP_DP_SECP521R1_ENABLED
conf unset MBEDTLS_ECP_DP_SECP192K1_ENABLED
conf unset MBEDTLS_ECP_DP_SECP224K1_ENABLED
conf unset MBEDTLS_ECP_DP_SECP256K1_ENABLED
conf unset MBEDTLS_ECP_DP_BP256R1_ENABLED
conf unset MBEDTLS_ECP_DP_BP384R1_ENABLED
conf unset MBEDTLS_ECP_DP_BP512R1_ENABLED
conf unset MBEDTLS_PK_PARSE_EC_EXTENDED

conf unset MBEDTLS_AESNI_C
conf unset MBEDTLS_ARC4_C
conf unset MBEDTLS_BLOWFISH_C
conf unset MBEDTLS_CAMELLIA_C
conf unset MBEDTLS_DES_C
conf unset MBEDTLS_DHM_C
conf unset MBEDTLS_GENPRIME
conf unset MBEDTLS_MD5_C
conf unset MBEDTLS_PADLOCK_C
conf unset MBEDTLS_PEM_WRITE_C
conf unset MBEDTLS_PKCS5_C
conf unset MBEDTLS_PKCS12_C
conf unset MBEDTLS_RIPEMD160_C
conf unset MBEDTLS_SHA1_C
conf unset MBEDTLS_XTEA_C

conf unset MBEDTLS_X509_RSASSA_PSS_SUPPORT

conf unset MBEDTLS_X509_CSR_PARSE_C
conf unset MBEDTLS_X509_CREATE_C
conf unset MBEDTLS_X509_CRT_WRITE_C
conf unset MBEDTLS_X509_CSR_WRITE_C

conf unset MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
conf unset MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
conf unset MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
conf unset MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
conf unset MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
conf unset MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
conf unset MBEDTLS_SSL_FALLBACK_SCSV
conf unset MBEDTLS_SSL_CBC_RECORD_SPLITTING
conf unset MBEDTLS_SSL_PROTO_SSL3
conf unset MBEDTLS_SSL_PROTO_TLS1
conf unset MBEDTLS_SSL_PROTO_TLS1_1
conf unset MBEDTLS_SSL_TRUNCATED_HMAC
