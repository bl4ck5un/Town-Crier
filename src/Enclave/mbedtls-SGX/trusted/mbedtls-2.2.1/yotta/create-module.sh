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

# relative to the script's directory
TREE=..
DEST=module

# make sure we're running in our own directory
if [ -f create-module.sh ]; then :; else
    cd $( dirname $0 )
    if [ -f create-module.sh ]; then :; else
        echo "Please run the script from is directory." >&2
        exit 1
    fi
fi

# use a temporary directory to build the module, then rsync to DEST
# this allows touching only new files, for more efficient re-builds
TMP=$DEST-tmp
rm -rf $TMP

mkdir -p $TMP/mbedtls $TMP/source
cp $TREE/include/mbedtls/*.h $TMP/mbedtls
cp $TREE/library/*.c $TMP/source

# temporary, should depend on external module later
cp data/entropy_hardware_poll.c $TMP/source
cp data/target_config.h $TMP/mbedtls

data/adjust-config.sh $TREE/scripts/config.pl $TMP/mbedtls/config.h

mkdir -p $TMP/test
cp -r data/example-* $TMP/test
# later we should have the generated test suites here too

cp data/module.json $TMP
cp data/README.md $TMP

cp ../LICENSE $TMP
if [ -f ../apache-2.0.txt ]; then cp ../apache-2.0.txt $TMP; fi

mkdir -p $DEST
rsync -cr --delete --exclude build --exclude yotta_\* $TMP/ $DEST/
rm -rf $TMP

echo "mbed TLS yotta module created in '$PWD/$DEST'."
