#
#
#
#
#
#
#
#
#
#
#

#include(PkgConfig)
include(FindPackageHandleStandardArgs)

#pkg_check_modules(PC_OpenImageIO
#    QUIET
#    OpenImageIO
#)

find_path(OpenImageIO_INCLUDE_DIR
    NAMES
        OpenImageIO/imageio.h
    PATHS
        ${PC_OpenImageIO_INCLUDE_DIRS}
    PATH_SUFFIXES
        include
)

find_library(OpenImageIO_LIBRARY
    NAMES
        OpenImageIO
    PATHS
        ${PC_OpenImageIO_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib64
        lib
)

find_package_handle_standard_args(OpenImageIO
    FOUND_VAR
        OpenImageIO_FOUND
    REQUIRED_VARS
        OpenImageIO_INCLUDE_DIR
        OpenImageIO_LIBRARY
    VERSION_VAR
        OpenImageIO_VERSION
)

if(OpenImageIO_FOUND)
    add_library(LibOpenImageIO
        SHARED
        IMPORTED
    )

    set_target_properties(LibOpenImageIO
        PROPERTIES
        IMPORTED_LOCATION
            ${OpenImageIO_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES
            ${OpenImageIO_INCLUDE_DIR}
    )
endif()
