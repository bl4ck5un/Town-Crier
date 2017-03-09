#/bin/sh
set -ex
sudo apt-get update -qq
sudo apt-get install -qq libcurl4-openssl-dev libjsoncpp-dev libargtable2-dev libgnutls-dev libgcrypt11-dev valgrind wget gcc-5 g++-5
tmp=$(mktemp -d)
pushd $tmp
wget http://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.52.tar.gz
tar -xvf libmicrohttpd-0.9.52.tar.gz
cd libmicrohttpd-0.9.52
./configure && make
sudo make install && sudo ldconfig
cd ..
git clone git://github.com/cinemast/libjson-rpc-cpp.git
mkdir -p libjson-rpc-cpp/build
cd libjson-rpc-cpp/build
cmake -DCOMPILE_TESTS=NO .. && make
sudo make install
sudo ldconfig          #only required for linux
popd
