PREREQUISITE
----------------

**Be sure to have SGX SDK installed and properly configured.**

Please refer to [linux-sgx](https://github.com/01org/linux-sgx) 
for installation and configuration instructions.

Third party dependencies
============================

- `CMake`: Town Crier uses CMake as the building tool. Please obtain it from your distributor.
- `libjsoncpp`: In ubuntu 16.04: `sudo apt-get install libjsoncpp-dev`
- [`libjson-rpc-cpp`](https://github.com/cinemast/libjson-rpc-cpp)
- sqlite3: `sudo apt-get install libsqlite3-0 libsqlite3-dev`
- google-test: see https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/ for a reference.

Build
-----

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

Test
----

After you build and install Town Crier, run `build/test_main`
to launch test (powered by GTest).
