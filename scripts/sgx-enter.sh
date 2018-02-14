#!/bin/bash

ROOTDIR=$( cd "$( dirname "${BASH_SOURCE[0]}")/.." && pwd )

tc_dev_image=bl4ck5un/tc-sgx-sdk
tc_dev_shell=bash

which docker >/dev/null || {
  echo "ERROR: Please install Docker first."
  exit 1
}

# Start SGX Rust Docker container.
docker run --rm -t -i \
  --name "tc-devel" \
  -v ${ROOTDIR}/src:/code \
  -e "TC_BUILD_CONFIG=Prerelease" \
  -e "SGX_SDK=/opt/intel/sgxsdk" \
  -w /build \
  -p 8123:8123 \
  "$tc_dev_image" \
  /usr/bin/env $tc_dev_shell
