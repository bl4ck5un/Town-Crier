# Try to find libodb and libodb-sqlite
#  LIBODB_FOUND - system has sqlite
#  LIBODB_INCLUDE_DIRS - the sqlite include directory
#  LIBODB_LIBRARIES - Link these to use sqlite
#

IF(APPLE)
    set(EXT dylib)
ELSE()
    set(EXT so)
ENDIF(APPLE)

FIND_PATH(LIBODB_INCLUDE_DIRS
        odb/core.hxx
        PATHS /usr/include/ /usr/local/include)
FIND_PATH(LIBODB_INCLUDE_DIRS
        odb/sqlite/database.hxx
        PATHS /usr/include/ /usr/local/include)
FIND_LIBRARY(LIBODB_LIBRARY libodb.${EXT}
        PATHS /usr/lib/odb /usr/local/lib)
FIND_LIBRARY(LIBODB_SQLITE_LIBRARY libodb-sqlite-2.4.${EXT}
        PATHS /usr/lib/odb /usr/local/lib)


# handle the QUIETLY and REQUIRED arguments and set LibODB_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibODB DEFAULT_MSG
    LIBODB_INCLUDE_DIRS LIBODB_LIBRARY LIBODB_SQLITE_LIBRARY)

set (LIBODB_LIBRARIES ${LIBODB_LIBRARY} ${LIBODB_SQLITE_LIBRARY})
MARK_AS_ADVANCED(LIBODB_LIBRARIES LIBODB_INCLUDE_DIRS)
