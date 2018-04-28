#!/usr/bin/env bash

CORE_IMAGE=bl4ck5un/tc-core
BUILD=Release

docker build --rm --force-rm --no-cache -t ${CORE_IMAGE}:${BUILD} .