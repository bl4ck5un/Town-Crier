#!/usr/bin/env bash

make all

docker push bl4ck5un/tc-core:Debug
docker push bl4ck5un/tc-core:Prerelease
docker push bl4ck5un/tc-relay
