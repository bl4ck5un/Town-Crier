set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set a default build type if none was specified
set(default_build_type "Debug")

if(DEFINED ENV{TC_BUILD_CONFIG})
    set(CMAKE_BUILD_TYPE $ENV{SGX_CONFIG})
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "Prerelease")
endif()
