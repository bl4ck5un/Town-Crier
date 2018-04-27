FROM bl4ck5un/tc-sgx-sdk
MAINTAINER Fan Zhang <bl4ck5unxx@gmail.com>

ARG BUILD=Debug
ARG MAKE_FLAGS=-j

COPY . /code

RUN mkdir /tc && \
    cp /code/dockerfiles/deployment/Dockerfile.runtime /tc/Dockerfile && \
    mv /code/dockerfiles/deployment/sign_enclave.sh \
       /code/dockerfiles/deployment/build_release_docker.sh \
       /code/dockerfiles/deployment/supervisord.conf /tc
RUN cd /code && \
    make -C src/Enclave/mbedtls-SGX && \
    mkdir -p tmp && \
    cd tmp && \
    cmake -DCMAKE_BUILD_TYPE=$BUILD -DCMAKE_INSTALL_PREFIX=/tc .. && \
    make $MAKE_FLAGS && \
    make install && \
    cd ..

# This is a builder container which outputs the contents of the package
# on standard output. This enables the runtime and the builder container
# to be different and reduces the image size considerably.
WORKDIR /tc
CMD tar cvzhf - .
