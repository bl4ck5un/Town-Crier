#!/usr/bin/env bash

echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
docker push bl4ck5un/tc-core:Debug
docker push bl4ck5un/tc-core:Prerelease
docker push bl4ck5un/tc-relay
