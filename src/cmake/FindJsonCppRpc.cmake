# Try to find JsonRpcCpp
#  JSONRPCCPP_FOUND - system has sqlite
#  JSONRPCCPP_INCLUDE_DIRS - the sqlite include directory
#  JSONRPCCPP_LIBRARIES - Link these to use sqlite

IF(APPLE)
    set(EXT dylib)
ELSE()
    set(EXT so)
ENDIF(APPLE)

FIND_PATH(JSONRPCCPP_INCLUDE_DIRS jsonrpccpp/version.h)
FIND_LIBRARY(JSONRPCCPP_SERVER_LIBRARY libjsonrpccpp-server.${EXT})
FIND_LIBRARY(JSONRPCCPP_CLIENT_LIBRARY libjsonrpccpp-client.${EXT})
FIND_LIBRARY(JSONRPCCPP_COMMON_LIBRARY libjsonrpccpp-common.${EXT})

# handle the QUIETLY and REQUIRED arguments and set JSONRPCCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSONRPCCPP DEFAULT_MSG
    JSONRPCCPP_INCLUDE_DIRS
    JSONRPCCPP_SERVER_LIBRARY
    JSONRPCCPP_CLIENT_LIBRARY
    JSONRPCCPP_COMMON_LIBRARY)

set(JSONRPCCPP_LIBRARIES 
    ${JSONRPCCPP_SERVER_LIBRARY} 
    ${JSONRPCCPP_CLIENT_LIBRARY} 
    ${JSONRPCCPP_COMMON_LIBRARY})

MARK_AS_ADVANCED(JSONRPCCPP_LIBRARIES JSONRPCCPP_INCLUDE_DIRS)
