#!/bin/bash -ex

TEMP=$(mktemp -d /tmp/tc.XXXXXXXX)
pushd $TEMP
wget http://www.codesynthesis.com/download/odb/2.4/libodb-2.4.0.tar.gz
wget http://www.codesynthesis.com/download/odb/2.4/libodb-sqlite-2.4.0.tar.gz
wget http://www.codesynthesis.com/download/odb/2.4/odb_2.4.0-1_amd64.deb

sudo dpkg -i odb_2.4.0-1_amd64.deb
tar xvf libodb-2.4.0.tar.gz
tar xvf libodb-sqlite-2.4.0.tar.gz

cd libodb-2.4.0
./configure 
make -j
sudo make install
cd ..

cd libodb-sqlite-2.4.0
./configure
make -j
sudo make install
cd ..

popd
