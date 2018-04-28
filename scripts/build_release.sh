#!/usr/bin/env bash

# exit on error
set -e

BASE_DIR=$(cd $PWD/.. && pwd)

pushd ${BASE_DIR} > /dev/null

docker build \
    --build-arg BUILD=Release \
    --build-arg MAKE_FLAGS=-j \
    --force-rm \
    -t tc/core-builder \
    -f dockerfiles/deployment/Dockerfile.build .

TEMP_DIR=${BASE_DIR}release

rm -rf ${TEMP_DIR} && mkdir -p ${TEMP_DIR}
docker run --rm tc/core-builder | tar xvzf - -C ${TEMP_DIR}

echo "artifact written to ${TEMP_DIR}"
echo "sign the enclave and run ${TEMP_DIR}/build_release_docker.sh"

popd > /dev/null
