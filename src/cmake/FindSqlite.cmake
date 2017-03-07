# - Try to find the sqlite library
# Once done this will define
#
#  SQLITE_FOUND - system has sqlite
#  SQLITE_INCLUDE_DIRS - the sqlite include directory
#  SQLITE_LIBRARIES - Link these to use sqlite
#
# Define SQLITE_MIN_VERSION for which version desired.
#

SET(SQLITE_MIN_VERSION 3)

FIND_PATH(SQLITE_INCLUDE_DIRS sqlite${SQLITE_MIN_VERSION}.h)
FIND_LIBRARY(SQLITE_LIBRARIES sqlite${SQLITE_MIN_VERSION})

# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sqlite DEFAULT_MSG
    SQLITE_INCLUDE_DIRS SQLITE_LIBRARIES)
MARK_AS_ADVANCED(SQLITE_LIBRARIES SQLITE_INCLUDE_DIRS)

