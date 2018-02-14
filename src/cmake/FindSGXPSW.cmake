# Try to find SGX SDK
#  SGXSuite_FOUND - system has SGX SDK/PSW installed
#  SGXSDK_INCLUDE_DIRS - the SGX SDK include directory
#  SGXSDK_LIBRARY_DIRS - the SGX SDK libraries directory
#  SGX_APP_LIBRARIES   - the untrusted runtime support libraries
#

FIND_PATH(SGXSDK_INCLUDE_DIRS sgx.h "${SGX_SDK}/include")

# force find_package to scan
UNSET(SGX_URTS_LIBRARY CACHE)
UNSET(SGX_USVC_LIBRARY CACHE)
UNSET(SGXSDK_LIBRARY_DIRS CACHE)

if (SGX_MODE MATCHES HW)
    FIND_LIBRARY(SGX_URTS_LIBRARY lib${SGX_URTS_LIB}.so /usr/lib)
    FIND_LIBRARY(SGX_USVC_LIBRARY lib${SGX_USVC_LIB}.so /usr/lib)
    FIND_PATH(SGXSDK_LIBRARY_DIRS lib${SGX_USVC_LIB}.so /usr/lib)
else()
    FIND_LIBRARY(SGX_URTS_LIBRARY lib${SGX_URTS_LIB}.so ${SGX_SDK}/lib64)
    FIND_LIBRARY(SGX_USVC_LIBRARY lib${SGX_USVC_LIB}.so ${SGX_SDK}/lib64)
    FIND_PATH(SGXSDK_LIBRARY_DIRS lib${SGX_USVC_LIB}.so ${SGX_SDK}/lib64)
endif()


# handle the QUIETLY and REQUIRED arguments and set SGXSuite_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SGXSuite DEFAULT_MSG
        SGXSDK_INCLUDE_DIRS
        SGXSDK_LIBRARY_DIRS
        SGX_URTS_LIBRARY
        SGX_USVC_LIBRARY)

MARK_AS_ADVANCED(SGXSDK_LIBRARIES SGXSDK_INCLUDE_DIRS)
