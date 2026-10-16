include_guard(GLOBAL)

function(engine_set_project_warnings target_name)
    target_compile_options(
        ${target_name}
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            # Warn when a user-declared destructor leaves implicit copies in place.
            # It also blocks implicit moves, so std::move may copy an owning type.
            # -Wall and -Wextra do not include this check.
            -Wdeprecated-copy-dtor
    )
    if(ENGINE_WARNINGS_AS_ERRORS)
        target_compile_options(${target_name} INTERFACE -Werror)
    endif()
endfunction()
