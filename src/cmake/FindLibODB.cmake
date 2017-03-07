# Try to find libodb and libodb-sqlite
#  LIBODB_FOUND - system has sqlite
#  LIBODB_INCLUDE_DIRS - the sqlite include directory
#  LIBODB_LIBRARIES - Link these to use sqlite
#

FIND_PATH(LIBODB_INCLUDE_DIRS odb/core.hxx )
FIND_PATH(LIBODB_INCLUDE_DIRS odb/sqlite/database.hxx)
FIND_LIBRARY(LIBODB_LIBRARY libodb.so)
FIND_LIBRARY(LIBODB_SQLITE_LIBRARY libodb-sqlite-2.4.so)


# handle the QUIETLY and REQUIRED arguments and set LibODB_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibODB DEFAULT_MSG
    LIBODB_INCLUDE_DIRS LIBODB_LIBRARY LIBODB_SQLITE_LIBRARY)

set (LIBODB_LIBRARIES ${LIBODB_LIBRARY} ${LIBODB_SQLITE_LIBRARY})
MARK_AS_ADVANCED(LIBODB_LIBRARIES LIBODB_INCLUDE_DIRS)
