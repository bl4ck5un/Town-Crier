FROM ubuntu:16.04
MAINTAINER Fan Zhang <bl4ck5unxx@gmail.com>

RUN apt-get -qq update
RUN apt-get -qq install -y build-essential automake autoconf libtool
RUN apt-get -qq install -y cmake libjsoncpp-dev libjsonrpccpp-dev libjsonrpccpp-tools libsqlite3-0 libsqlite3-dev libboost-all-dev libmicrohttpd-dev libcurl4-openssl-dev odb
RUN apt-get -qq install -y wget

RUN mkdir sgx
RUN wget -O sgx/sgx_sdk.bin https://download.01.org/intel-sgx/linux-2.1.1/ubuntu64-server/sgx_linux_x64_sdk_2.1.101.42529.bin
RUN chmod a+x sgx/sgx_sdk.bin
RUN echo -e 'no\n/opt/intel' | $PWD/sgx/sgx_sdk.bin
RUN echo 'source /opt/intel/sgxsdk/environment' >> $PWD/.bashrc
