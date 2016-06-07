include ../Makefile.defs

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

App_Include_Paths := \
	-I../Common \
	-I$(SGX_SDK)/include \
	-I../mbedtls-SGX/lib/include \
	-I/usr/include/jsoncpp \
	-I../jsonrpc-cpp-0.6/include

App_C_Files := $(wildcard *.c) $(wildcard ../Common/*.c)
App_C_Objects := $(App_C_Files:.c=.o)
App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths)

App_Cpp_Files := App.cpp $(wildcard *.cpp) $(wildcard ../Common/*.cpp)
App_Cpp_Objects := $(App_Cpp_Files:.cpp=.o)
App_Cpp_Flags := $(App_C_Flags) -std=c++11


ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) \
	-L../mbedtls-SGX/lib -lmbedtls_sgx_u \
	-lsqlite3 -ljsonrpc-cpp -ljsoncpp

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_Name := tc

.PHONY: all

all: $(App_Name)

Enclave_u.c: $(SGX_EDGER8R) ../Enclave/Enclave.edl
	@$(SGX_EDGER8R) --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

Enclave_u.o: Enclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

%.o: %.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

%.o: %.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

$(App_Name): Enclave_u.o $(App_Cpp_Objects) $(App_C_Objects)
	$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

