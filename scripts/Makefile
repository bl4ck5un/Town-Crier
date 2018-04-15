BASE_DIR=$(shell pwd)/..

BUILD=Debug

RELAY_IMAGE=bl4ck5un/tc-relay
CORE_IMAGE=bl4ck5un/tc-core

.PHONY: all up up-debug

all: relay core core-debug

relay:
	cd ${BASE_DIR}/python-relay && \
	docker build --rm --force-rm -t ${RELAY_IMAGE} . && \
	cd ..

core:
	$(MAKE) BUILD=Prerelease build

core-debug:
	$(MAKE) BUILD=Debug build

build:
	cd ${BASE_DIR} && \
	docker build \
		--build-arg BUILD=${BUILD} \
		--build-arg MAKE_FLAGS=-j \
		--force-rm \
		-t tc/core-builder \
		-f dockerfiles/deployment/Dockerfile.build . && \
	docker run --rm tc/core-builder | docker build --rm --force-rm -t ${CORE_IMAGE}:${BUILD} - && \
	cd ..

update:
	docker-compose -f docker-compose.rel.yml pull
	docker-compose -f docker-compose.debug.yml pull

up:
	docker-compose -f docker-compose.rel.yml up -d

up-bash:
	docker-compose -f ./docker-compose.rel.yml exec tc-core bash

up-debug:
	docker-compose -f docker-compose.debug.yml up
