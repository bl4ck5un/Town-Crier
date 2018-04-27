#!/usr/bin/env bash

CORE_IMAGE=bl4ck5un/tc-core
BUILD=Release

docker run --rm tc/core-builder | docker build --rm --force-rm -t ${CORE_IMAGE}:${BUILD} .
