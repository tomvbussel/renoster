function(make_plugin plugin srcs)
    add_library(${plugin}
        SHARED
        ${srcs}
    )

    target_compile_features(${plugin}
        PRIVATE
            cxx_std_17
    )

    set_property(
        TARGET
            ${plugin}
        PROPERTY
            POSITION_INDEPENDENT_CODE ON
    )

    set_property(
        TARGET
            ${plugin}
        PROPERTY
            PREFIX ""
    )

    target_link_libraries(${plugin}
        PRIVATE
            LibRenoster
            ${ARGV2}
    )
endfunction(make_plugin)
