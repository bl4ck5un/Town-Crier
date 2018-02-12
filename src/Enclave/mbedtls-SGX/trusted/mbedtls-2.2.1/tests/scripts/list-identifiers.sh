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

if [ -d include/mbedtls ]; then :; else
    echo "$0: must be run from root" >&2
    exit 1
fi

HEADERS=$( ls include/mbedtls/*.h | egrep -v 'compat-1\.3\.h|bn_mul' )

rm -f identifiers

grep '^[^ /#{]' $HEADERS | \
    sed -e 's/^[^:]*://' | \
    egrep -v '^(extern "C"|(typedef )?(struct|enum)( {)?$|};?$)' \
    > _decls

if true; then
sed -n -e 's/.* \**\([a-zA-Z_][a-zA-Z0-9_]*\)(.*/\1/p' \
       -e 's/.*(\*\(.*\))(.*/\1/p' _decls
grep -v '(' _decls | sed -e 's/\([a-zA-Z0-9_]*\)[;[].*/\1/' -e 's/.* \**//'
fi > _identifiers

if [ $( wc -l < _identifiers ) -eq $( wc -l < _decls ) ]; then
    rm _decls
    egrep -v '^(u?int(16|32|64)_t)$' _identifiers | sort > identifiers
    rm _identifiers
else
    echo "$0: oops, lost some identifiers" 2>&1
    exit 1
fi

wc -l identifiers
