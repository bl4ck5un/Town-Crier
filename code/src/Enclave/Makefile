include ../Makefile.defs

ENCLAVE_FLAGS := -DENCLAVE_STD_ALT

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

Enclave_Include_Paths := \
	-I../Common \
	-I../mbedtls-SGX/lib/include \
	-I../mbedtls-2.2.1/include \
	-I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc \
	-I$(SGX_SDK)/include/stlport

Enclave_C_Files := $(wildcard *.c) $(wildcard ../Common/*.c)
Enclave_C_Objects := $(Enclave_C_Files:.c=.o)
Enclave_C_Flags := $(ENCLAVE_FLAGS) $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector \
	$(Enclave_Include_Paths)

Enclave_Cpp_Files := $(wildcard *.cpp) $(wildcard ../Common/*.cpp)
Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)
Enclave_Cpp_Flags := $(Enclave_C_Flags) -std=c++03 -nostdinc++

Enclave_Objects := $(Enclave_C_Objects) $(Enclave_Cpp_Objects)

Enclave_Link_Flags := $(SGX_COMMON_CFLAGS) \
	-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) -L../mbedtls-SGX/lib \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lmbedtls_sgx -lsgx_tstdc -lsgx_tstdcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif

.PHONY: all

all: $(Signed_Enclave_Name)

Enclave_t.c: $(SGX_EDGER8R) Enclave.edl
	@$(SGX_EDGER8R) --trusted Enclave.edl \
		--search-path . \
		--search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

Enclave_t.o: Enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

%.o: %.cpp
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

%.o: %.c
	@$(CXX) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC  <=  $<"

$(Enclave_Name): Enclave_t.o $(Enclave_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags) 
	@echo "LINK =>  $@"

$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key Enclave_private.pem \
		-enclave $(Enclave_Name) \
		-config $(Enclave_Config_File) \
		-out $@ 
	@echo "SIGN =>  $@"

clean: 
	rm -rf *.o *.so
