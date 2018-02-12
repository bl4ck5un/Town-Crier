FROM ubuntu:16.04
MAINTAINER Fan Zhang <bl4ck5unxx@gmail.com>

RUN apt-get -q update
RUN apt-get -q install -y build-essential automake autoconf libtool \
    cmake libjsoncpp-dev libjsonrpccpp-dev libjsonrpccpp-tools libsqlite3-0 libsqlite3-dev \
    libboost-all-dev libmicrohttpd-dev libcurl4-openssl-dev odb wget

RUN mkdir /root/sgx && \
    wget -O /root/sgx/sgx_linux_ubuntu16.04.1_x64_psw_2.0.100.40950.bin https://download.01.org/intel-sgx/linux-2.0/sgx_linux_ubuntu16.04.1_x64_psw_2.0.100.40950.bin && \
    wget -O /root/sgx/sgx_linux_ubuntu16.04.1_x64_sdk_2.0.100.40950.bin https://download.01.org/intel-sgx/linux-2.0/sgx_linux_ubuntu16.04.1_x64_sdk_2.0.100.40950.bin && \
    cd /tmp && \
    git clone https://github.com/01org/dynamic-application-loader-host-interface.git jhi && \
    cd jhi && mkdir build && cd build && cmake .. && make && make install && \
    cd /root/sgx && \
    chmod +x /root/sgx/sgx_linux_ubuntu16.04.1_x64_psw_2.0.100.40950.bin && \
    /root/sgx/sgx_linux_ubuntu16.04.1_x64_psw_2.0.100.40950.bin && \
    chmod +x /root/sgx/sgx_linux_ubuntu16.04.1_x64_sdk_2.0.100.40950.bin && \
    echo -e 'no\n/opt/intel' | /root/sgx/sgx_linux_ubuntu16.04.1_x64_sdk_2.0.100.40950.bin && \
    echo 'source /opt/sgxsdk/environment' >> /root/.bashrc && \
    systemctl enable jhi

RUN wget -O /root/sgx_2.0.tar.gz https://github.com/01org/linux-sgx/archive/sgx_2.0.tar.gz && \
    cd /root && tar xzf sgx_2.0.tar.gz && \
    cd /root/linux-sgx-sgx_2.0 && \
    /root/linux-sgx-sgx_2.0/download_prebuilt.sh && \
    cd /root/linux-sgx-sgx_2.0 && make && \
    cp /root/linux-sgx-sgx_2.0/build/linux/libsgx_tstdc.a /opt/sgxsdk/lib64/libsgx_tstdc.a
