#!/bin/bash
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
VERSION=""
SOVERSION=""

# Parse arguments
#
until [ -z "$1" ]
do
  case "$1" in
    --version)
      # Version to use
      shift
      VERSION=$1
      ;;
    --so-crypto)
      shift
      SO_CRYPTO=$1
      ;;
    --so-x509)
      shift
      SO_X509=$1
      ;;
    --so-tls)
      shift
      SO_TLS=$1
      ;;
    -v|--verbose)
      # Be verbose
      VERBOSE="1"
      ;;
    -h|--help)
      # print help
      echo "Usage: $0"
      echo -e "  -h|--help\t\tPrint this help."
      echo -e "  --version <version>\tVersion to bump to."
      echo -e "  --so-crypto <version>\tSO version to bump libmbedcrypto to."
      echo -e "  --so-x509 <version>\tSO version to bump libmbedx509 to."
      echo -e "  --so-tls <version>\tSO version to bump libmbedtls to."
      echo -e "  -v|--verbose\t\tVerbose."
      exit 1
      ;;
    *)
      # print error
      echo "Unknown argument: '$1'"
      exit 1
      ;;
  esac
  shift
done

if [ "X" = "X$VERSION" ];
then
  echo "No version specified. Unable to continue."
  exit 1
fi

[ $VERBOSE ] && echo "Bumping VERSION in library/CMakeLists.txt"
sed -e "s/ VERSION [0-9.]\{1,\}/ VERSION $VERSION/g" < library/CMakeLists.txt > tmp
mv tmp library/CMakeLists.txt

if [ "X" != "X$SO_CRYPTO" ];
then
  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedcrypto in library/CMakeLists.txt"
  sed -e "/mbedcrypto/ s/ SOVERSION [0-9]\{1,\}/ SOVERSION $SO_CRYPTO/g" < library/CMakeLists.txt > tmp
  mv tmp library/CMakeLists.txt

  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedcrypto in library/Makefile"
  sed -e "s/SOEXT_CRYPTO=so.[0-9]\{1,\}/SOEXT_CRYPTO=so.$SO_CRYPTO/g" < library/Makefile > tmp
  mv tmp library/Makefile
fi

if [ "X" != "X$SO_X509" ];
then
  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedx509 in library/CMakeLists.txt"
  sed -e "/mbedx509/ s/ SOVERSION [0-9]\{1,\}/ SOVERSION $SO_X509/g" < library/CMakeLists.txt > tmp
  mv tmp library/CMakeLists.txt

  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedx509 in library/Makefile"
  sed -e "s/SOEXT_X509=so.[0-9]\{1,\}/SOEXT_X509=so.$SO_X509/g" < library/Makefile > tmp
  mv tmp library/Makefile
fi

if [ "X" != "X$SO_TLS" ];
then
  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedtls in library/CMakeLists.txt"
  sed -e "/mbedtls/ s/ SOVERSION [0-9]\{1,\}/ SOVERSION $SO_TLS/g" < library/CMakeLists.txt > tmp
  mv tmp library/CMakeLists.txt

  [ $VERBOSE ] && echo "Bumping SOVERSION for libmbedtls in library/Makefile"
  sed -e "s/SOEXT_TLS=so.[0-9]\{1,\}/SOEXT_TLS=so.$SO_TLS/g" < library/Makefile > tmp
  mv tmp library/Makefile
fi

[ $VERBOSE ] && echo "Bumping VERSION in include/mbedtls/version.h"
read MAJOR MINOR PATCH <<<$(IFS="."; echo $VERSION)
VERSION_NR="$( printf "0x%02X%02X%02X00" $MAJOR $MINOR $PATCH )"
cat include/mbedtls/version.h |                                    \
    sed -e "s/_VERSION_MAJOR .\{1,\}/_VERSION_MAJOR  $MAJOR/" |    \
    sed -e "s/_VERSION_MINOR .\{1,\}/_VERSION_MINOR  $MINOR/" |    \
    sed -e "s/_VERSION_PATCH .\{1,\}/_VERSION_PATCH  $PATCH/" |    \
    sed -e "s/_VERSION_NUMBER .\{1,\}/_VERSION_NUMBER         $VERSION_NR/" |    \
    sed -e "s/_VERSION_STRING .\{1,\}/_VERSION_STRING         \"$VERSION\"/" |    \
    sed -e "s/_VERSION_STRING_FULL .\{1,\}/_VERSION_STRING_FULL    \"mbed TLS $VERSION\"/" \
    > tmp
mv tmp include/mbedtls/version.h

[ $VERBOSE ] && echo "Bumping version in tests/suites/test_suite_version.data"
sed -e "s/version:\".\{1,\}/version:\"$VERSION\"/g" < tests/suites/test_suite_version.data > tmp
mv tmp tests/suites/test_suite_version.data

[ $VERBOSE ] && echo "Bumping version in yotta/data/module.json"
sed -e "s/\"version\": \".\{1,\}\"/\"version\": \"$VERSION\"/g" < yotta/data/module.json > tmp
mv tmp yotta/data/module.json

[ $VERBOSE ] && echo "Bumping PROJECT_NAME in doxygen/mbedtls.doxyfile and doxygen/input/doc_mainpage.h"
for i in doxygen/mbedtls.doxyfile doxygen/input/doc_mainpage.h;
do
  sed -e "s/mbed TLS v[0-9\.]\{1,\}/mbed TLS v$VERSION/g" < $i > tmp
  mv tmp $i
done

[ $VERBOSE ] && echo "Re-generating library/error.c"
scripts/generate_errors.pl

[ $VERBOSE ] && echo "Re-generating library/version_features.c"
scripts/generate_features.pl

[ $VERBOSE ] && echo "Re-generating visualc files"
scripts/generate_visualc_files.pl
