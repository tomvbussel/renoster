add_executable(renoster main.cpp)

target_compile_features(renoster
    PRIVATE
        cxx_std_17
)

target_include_directories(renoster
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(renoster
    PRIVATE
        LibRenoster
        Boost::boost
        Boost::program_options
)
