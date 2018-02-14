#
# Copyright 2013 Produban
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Tries to find Log4cxx Client headers and libraries.
#
# Usage of this module as follows:
#
#  find_package(Log4CXX)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  LOG4CXX_ROOT_DIR  Set this variable to the root installation of
#                  	 Log4cxx Library if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  LOG4CXX_FOUND              System has Log4cxx libs/headers
#  LOG4CXX_LIBRARIES          The Log4cxx libraries
#  LOG4CXX_INCLUDE_DIRS       The location of Log4cxx headers

if (LOG4CXX_ROOT_DIR)
    message (STATUS "Root dir: ${LOG4CXX_ROOT_DIR}")
endif ()

find_package(PkgConfig)
pkg_check_modules(PC_LOG4CXX log4cxx)
set(LOG4CXX_DEFINITIONS ${PC_LOG4CXX_CFLAGS_OTHER})

find_path(LOG4CXX_INCLUDE_DIR
	NAMES
		log4cxx.h
    PATHS
		${LOG4CXX_ROOT_DIR}/include
        ${PC_LOG4CXX_INCLUDEDIR}
        ${PC_LOG4CXX_INCLUDE_DIRS}
    PATH_SUFFIXES
		log4cxx
)

find_library(LOG4CXX_LIBRARY
    NAMES
		log4cxx
    PATHS
		${LOG4CXX_ROOT_DIR}/lib
        ${PC_LOG4CXX_LIBDIR}
        ${PC_LOG4CXX_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LOG4CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LOG4CXX
	DEFAULT_MSG
    LOG4CXX_LIBRARY
	LOG4CXX_INCLUDE_DIR
)

mark_as_advanced(LOG4CXX_INCLUDE_DIR LOG4CXX_LIBRARY)

if (LOG4CXX_FOUND)
	set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})
	set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARY})

    get_filename_component(LOG4CXX_LIBRARY_DIR ${LOG4CXX_LIBRARY} PATH)
    get_filename_component(LOG4CXX_LIBRARY_NAME ${LOG4CXX_LIBRARY} NAME_WE)

    mark_as_advanced(LOG4CXX_LIBRARY_DIR LOG4CXX_LIBRARY_NAME)

	message (STATUS "Include directories: ${LOG4CXX_INCLUDE_DIRS}")
	message (STATUS "Libraries: ${LOG4CXX_LIBRARIES}")
endif ()
