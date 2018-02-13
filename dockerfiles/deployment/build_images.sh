#!/bin/bash -e

base_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )

pushd ${base_dir}

# Build the builder Docker image first.
docker build \
    --force-rm \
    -t tc/core-builder \
    -f dockerfiles/deployment/Dockerfile.build .
popd
