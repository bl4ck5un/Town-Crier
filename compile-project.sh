./src/utils/build_gtest.sh
rm -rf build/
mkdir build
cd build
cmake ..
make
make TestMain