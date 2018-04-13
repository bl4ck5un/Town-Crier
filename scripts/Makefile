BASE_DIR=$(shell pwd)/..

BUILD=Debug

.PHONY: all up up-debug

all: relay core debug-core

relay:
	cd ${BASE_DIR}/python-relay && \
	docker build --rm --force-rm -t tc/relay . && \
	cd ..

core:
	$(MAKE) BUILD=Prerelease build

debug-core:
	$(MAKE) BUILD=Debug build

build:
	cd ${BASE_DIR} && \
	docker build \
		--build-arg BUILD=${BUILD} \
		--build-arg MAKE_FLAGS=-j \
		--force-rm \
		-t tc/core-builder \
		-f dockerfiles/deployment/Dockerfile.build . && \
	docker run --rm tc/core-builder | docker build --rm --force-rm -t tc/core:${BUILD} - && \
	cd ..

up:
	docker-compose -f docker-compose.yml up

up-debug:
	docker-compose -f docker-compose.yml -f docker-compose.debug.yml up
