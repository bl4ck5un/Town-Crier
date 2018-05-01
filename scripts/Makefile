BASE_DIR=$(shell pwd)/..

BUILD=Debug

RELAY_IMAGE=bl4ck5un/tc-relay
CORE_IMAGE=bl4ck5un/tc-core

.PHONY: all up up-debug

all: relay core core-debug

relay: ${BASE_DIR}/python-relay/relay.py
	cd ${BASE_DIR}/python-relay && \
	docker build --rm --force-rm -t ${RELAY_IMAGE} . && \
	cd ..

core:
	$(MAKE) BUILD=Prerelease build

core-debug:
	$(MAKE) BUILD=Debug build

release:
	@./build_release.sh

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

push: relay core core-debug
	docker push ${CORE_IMAGE}:Debug
	docker push ${CORE_IMAGE}:Prerelease
	docker push ${RELAY_IMAGE}
