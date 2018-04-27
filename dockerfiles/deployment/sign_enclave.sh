#!/usr/bin/env bash

SGX_SDK=${SGX_SDK:-"/opt/intel/sgxsdk"}
SGX_ENCLAVE_SIGNER=${SGX_SDK}/bin/x64/sgx_sign

SIGNATURE_FILE=signature.hex
SIGNING_WORKING_DIR=signing

if [[ -f  ${SIGNATURE_FILE} ]]; then
pushd ${SIGNING_WORKING_DIR}
    ${SGX_ENCLAVE_SIGNER} catsig
        -enclave enclave.debug.so \
        -config Enclave.config.xml \
        -key release_pubkey.pem \
        -out enclave.release.so \
        -sig signature.hex \
        -unsigned enclave.hex
popd
else
    echo "nothing to do";
fi
