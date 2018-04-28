FROM bl4ck5un/tc-sgx-sdk
MAINTAINER Fan Zhang <bl4ck5unxx@gmail.com>

ENV PATH="/tc/bin:${PATH}"
ENV SGX_SDK=/opt/intel/sgxsdk
ENV LD_LIBRARY_PATH=${SGX_SDK}/sdk_libs

COPY . /tc

WORKDIR /tc

RUN apt-get update && apt-get install --yes supervisor
RUN ./sign_enclave.sh

EXPOSE 8123

COPY ./supervisord.conf /etc

ENTRYPOINT ["supervisord", "-n", "-c", "/etc/supervisord.conf"]
