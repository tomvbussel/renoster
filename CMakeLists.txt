cmake_minimum_required(VERSION 3.10)

project(renoster
    VERSION
        0.1
    DESCRIPTION
        TODO
    LANGUAGES
        CXX
)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(OpenImageIO
    REQUIRED
)

find_package(Boost
    REQUIRED 
    COMPONENTS
        filesystem
        program_options
        regex
        system
        thread
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include(plugins)

add_subdirectory(thirdparty)
add_subdirectory(src/librenoster)
add_subdirectory(src/plugins)
add_subdirectory(src/main)
add_subdirectory(src/tools)
add_subdirectory(test)
