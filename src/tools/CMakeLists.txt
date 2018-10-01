add_executable(obj2reno obj2reno.cpp)

target_compile_features(obj2reno
    PRIVATE
        cxx_std_17
)

target_include_directories(obj2reno
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(obj2reno
    PRIVATE
        LibRenoster
        Boost::boost
        Boost::program_options
)
