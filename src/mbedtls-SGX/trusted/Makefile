######## Project Settings ########

PROJECT_NAME=mbedtls_sgx
PROJECT_EDL=$(PROJECT_NAME).edl

Enclave_Name := lib$(PROJECT_NAME)_t.a
Enclave_Config_File := $(PROJECT_NAME).config.xml

MBEDTLS_VERSION=2.2.1
MBEDTLS_ROOT=mbedtls-$(MBEDTLS_VERSION)
MBEDTLS_HEADER_DIR=$(MBEDTLS_ROOT)/include
MBEDTLS_C_DIR=$(MBEDTLS_ROOT)/library

######## SGX SDK Settings ########
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64
SGX_DEBUG = 1

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O2
endif

SGX_COMMON_CFLAGS += -Wall

######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

Enclave_C_Files = $(PROJECT_NAME)_t.c $(wildcard $(MBEDTLS_C_DIR)/*.c) glue.c
Enclave_Include_Paths = -I$(MBEDTLS_HEADER_DIR) -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/stlport -I.

Enclave_C_Flags = $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Enclave_Include_Paths)
Enclave_Cpp_Flags = $(Enclave_C_Flags) -std=c++03 -nostdinc++

Enclave_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0

Enclave_C_Objects := $(Enclave_C_Files:.c=.o)


ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all

ifeq ($(Build_Mode), HW_RELEASE)
all: $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you link $(Enclave_Name) with your enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool. See User's Guide for more details."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: $(Enclave_Name) libmbedtls_sgx_u.a
endif


######## Enclave Objects ########

$(PROJECT_NAME)_t.c: $(SGX_EDGER8R) 
	@$(SGX_EDGER8R) --trusted $(PROJECT_EDL) --search-path $(MBEDTLS_HEADER_DIR) \
		--search-path $(SGX_SDK)/include
	@echo "GEN =>  $@"

%.o: %.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC <= $<"

$(Enclave_Name): $(PROJECT_NAME)_t.o $(Enclave_C_Objects)
	@ar rcs $@ $^
	@echo "LD => $@"


######## Untrusted parts ########

UNTRUSTED_SRC_FILES := ../untrusted/mbedtls_sgx_u_net.c ../untrusted/mbedtls_sgx_u_std.c
UNTRUSTED_OBJECTS := $(UNTRUSTED_SRC_FILES:.c=.u.o)

libmbedtls_sgx_u.a:	$(UNTRUSTED_OBJECTS)
	@ar rcs $@ $^
	@echo "LD => $@"

%.u.o: %.c
	@$(CC) -Imbedtls-2.2.1/include  -c -o $@ $<
	@echo "CC <= $<"

.PHONY: clean

clean:
	@rm -f $(Enclave_Name) $(Signed_Enclave_Name) \
		$(Enclave_C_Objects) $(UNTRUSTED_OBJECTS)\
		$(PROJECT_NAME)_t.* $(PROJECT_NAME)_u.*
