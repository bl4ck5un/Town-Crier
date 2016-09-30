PREREQUISITE
----------------

**Be sure to have SGX SDK installed and properly configured.**

Third party dependencies
============================

- CMake: Town Crier uses CMake as the building tool. Please obtain it from your
  distributor.
- `libjsoncpp`: sudo apt-get install libjsoncpp-dev
- [`libjson-rpc-cpp`](https://github.com/cinemast/libjson-rpc-cpp)
- sqlite3: `sudo apt-get install libsqlite3-0 libsqlite3-dev`
- google-test: see https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/ for a reference.
- http-parser: https://github.com/nodejs/http-parser. No package available yet. Can only be installed by compilcation.


Build
--------------

Build the SGX library first:

```
cd Enclave/mbedtls-SGX
make
```

Then build the Town Crier with CMake:


```
mkdir build
cmake ..
make
make install
```

Then the compiled binary will be installed at `build/tc`.
