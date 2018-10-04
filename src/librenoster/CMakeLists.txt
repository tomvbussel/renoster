find_package(BISON)
set(bison_cpp_output
    ${CMAKE_CURRENT_BINARY_DIR}/renoparse.cpp
)
set(bison_hpp_output
    ${CMAKE_CURRENT_BINARY_DIR}/renoparse.hpp
)
add_custom_command(
    OUTPUT
        ${bison_cpp_output}
        ${bison_hpp_output}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/io/renoparse.y
    COMMAND
        ${BISON_EXECUTABLE} -d -v -t
        --output=${bison_cpp_output}
        ${CMAKE_CURRENT_SOURCE_DIR}/io/renoparse.y
)

find_package(FLEX)
set(flex_cpp_output
    ${CMAKE_CURRENT_BINARY_DIR}/renolex.cpp
)
add_custom_command (
    OUTPUT
        ${flex_cpp_output}
    DEPENDS
        ${bison_hpp_output}
        ${CMAKE_CURRENT_SOURCE_DIR}/io/renolex.l
    COMMAND
        ${FLEX_EXECUTABLE}
        -o ${flex_cpp_output}
        ${CMAKE_CURRENT_SOURCE_DIR}/io/renolex.l
)

add_library (LibRenoster SHARED
    bsdf.cpp
    bvh.cpp
    camera.cpp
    film.cpp
    filmaccumulator.cpp
    filtertable.cpp
    geometry.cpp
    microfacet.cpp
    paramlist.cpp
    plugin.cpp
    primitive.cpp
    renderer.cpp
    reno.cpp
    renoparser.cpp
    sampling.cpp
    scene.cpp
    shading.cpp
    transform.cpp
    util/filesystem.cpp
    ${bison_cpp_output}
    ${flex_cpp_output}
)

target_compile_features(LibRenoster
    PUBLIC
        cxx_std_17
)

set_target_properties(LibRenoster
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)

target_include_directories(LibRenoster
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/librenoster
)

target_link_libraries(LibRenoster
    PUBLIC
        tinyformat
    PRIVATE
        LibOpenImageIO
        ${CMAKE_DL_LIBS}
        Boost::boost
        Boost::regex
        Boost::filesystem
)
