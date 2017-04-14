# Try to find SGX SDK
#  SGXSDK_FOUND - system has sqlite
#  SGXSDK_INCLUDE_DIRS - the sqlite include directory
#  SGXSDK_LIBRARIES - Link these to use sqlite
#

FIND_PATH(SGXSDK_INCLUDE_DIRS sgx.h "${SGX_SDK}/include")
FIND_LIBRARY(SGXSDK_LIBRARIES libsgx_urts.so "${SGX_SDK}/lib64")

# handle the QUIETLY and REQUIRED arguments and set LibODB_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SGXSDK DEFAULT_MSG
    SGXSDK_INCLUDE_DIRS SGXSDK_LIBRARIES)

MARK_AS_ADVANCED(SGXSDK_LIBRARIES SGXSDK_INCLUDE_DIRS)
