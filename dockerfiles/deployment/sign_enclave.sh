#!/usr/bin/env bash

SGX_SDK=${SGX_SDK:-"/opt/intel/sgxsdk"}
SGX_ENCLAVE_SIGNER=${SGX_SDK}/bin/x64/sgx_sign

SIGNING_WORKING_DIR=signing
SIGNATURE_FILE=${SIGNING_WORKING_DIR}/signature.hex

if [[ -f  ${SIGNATURE_FILE} ]]; then
pushd ${SIGNING_WORKING_DIR} > /dev/null
    ${SGX_ENCLAVE_SIGNER} catsig \
        -enclave libenclave.so \
        -config Enclave.config.xml \
        -key release_pubkey.pem \
        -out ../enclave/enclave.release.so \
        -sig signature.hex \
        -unsigned enclave.hex
popd > /dev/null
else
    echo "nothing to do";
fi
