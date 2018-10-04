add_executable(renoster_test
    bounds.cpp
    frame.cpp
)
target_link_libraries(renoster_test
    PRIVATE
        LibRenoster
        gtest_main
)
add_test(renoster_unit_test renoster_test)
