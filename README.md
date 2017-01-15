# Prerequisites 

## Intel Software Guardian eXtension (SGX)

Town Crier makes use of Intel SGX to provide security guarantees. Compiling TC
requires shared libraries provided by the SGX Software Development Kit (SDK),
which is freely available from Intel. SGX SDK provides two sets of SGX
libraries, a _hardware_ one and a _simulation_ one, with the only difference
(yet a significant one) being that the hardware libraries implement APIs with
real SGX hardware instructions while the simulation libraries just simulate
them in the user space. TC can be linked against either set of APIs.

If linked with the hardware libraries, the executable will run in the _hardware_
mode, which requires a SGX-enabled CPU to run.  If linked with the simulation
libraries, the executable will run as if it's an ordinary userspace program,
providing ABSOLUTELY NO SECURITY GUARANTEE. Therefore the simulation mode is
only meant for development and test purpose.

### Requisites for the simulation mode

Any system, not matter it has SGX hardware or not, can be used to develop Town
Crier and test it in the simulation mode Simulation actually makes debugging and
profiling easier and can be run on almost any Linux system.

To compile and run Town Crier in the simulation mode, one only needs to install
the SGX SDK, which can be obtained from
[here](https://01.org/intel-software-guard-extensions/downloads).

### Requisites for the hardware mode

To run TC on real SGX hardware, some dependency (SGX driver, SGX PSW, SGX SDK)
has to be installed and configured. Please refer to
[linux-sgx](https://github.com/01org/linux-sgx) for instructions.


## Libraries 

- `cmake`: >= 3.0
- `libjsoncpp` 
- [`libjson-rpc-cpp`](https://github.com/cinemast/libjson-rpc-cpp)
- sqlite3: 
- `boost`

On Ubuntu 16.04 LTS: 

```
sudo apt-get install cmake libjsoncpp-dev libjsonrpccpp-dev libjsonrpccpp-tools libsqlite3-0 libsqlite3-dev libboost-all-dev 
```

Other platforms have yet to be tested.

# Build

## Build without testing

Build the TLS library first:

```
make -C Enclave/mbedtls-SGX
```

Then build the Town Crier with CMake:

```
mkdir build
cd build
cmake ..
make tc
```

The compiled binary will be `build/tc`.

## Build with Tests (gtest)
----

To build tests, build vendored `gtest` first

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
