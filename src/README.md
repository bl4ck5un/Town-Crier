PREREQUISITE
----------------

SGX Software
============

## Run Town Crier in the simulation mode

Any system, not matter it has SGX hardware or not, can be used to develop Town Crier and test it in the simulation mode,
where no protection is indeed conferred. Simulation actually makes debugging and profiling easier and can be run on
almost any Linux system.

To compile and run Town Crier in the simulation mode, one only needs to install the SGX SDK, which can be obtained from
[here](https://01.org/intel-software-guard-extensions/downloads).

## Run Town Crier in **hardware** Mode

To run TC on real SGX hardware, some dependency (SGX driver, SGX PSW, SGX SDK) has to be installed and configured.
Please refer to [linux-sgx](https://github.com/01org/linux-sgx) for instructions.


Dependencies
============

- `cmake`: Town Crier uses CMake as the building tool. Please obtain it from your distributor.
- `libjsoncpp`: In ubuntu 16.04: `sudo apt-get install libjsoncpp-dev`
- [`libjson-rpc-cpp`](https://github.com/cinemast/libjson-rpc-cpp)
- sqlite3: `sudo apt-get install libsqlite3-0 libsqlite3-dev`
- `cfgparser`: In http://cfgparser.sourceforge.net/

Build
-----

Build the TLS library first:

```
cd Enclave/mbedtls-SGX
make
```

Then build the Town Crier with CMake:

```
mkdir build
cd build
cmake ..
make tc
```

The compiled binary will be `build/tc`.

Test
----

To build tests, one needs to build vendored `gtest`

```
./utils/build_gtest.sh
```

Then build self tests with CMake the same as building Town Crier

```
mkdir build
cd build
cmake ..
make TestMain
```


After you build and install Town Crier, run `build/TestMain` to test (powered by GTest).

Run
----

A configuration is needed to run tc.

We need to specify RPChost in this configuration.

```
cd build
echo "[RPC]" >> config
echo "RPChost = http://localhost:8200" >> config
./tc config
```
