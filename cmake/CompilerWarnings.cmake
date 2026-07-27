include_guard(GLOBAL)

function(engine_set_project_warnings target_name)
    if(MSVC)
        target_compile_options(
            ${target_name}
            INTERFACE /W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc
        )
        if(ENGINE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} INTERFACE /WX)
        endif()
    else()
        target_compile_options(
            ${target_name}
            INTERFACE
                -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
                -Wshadow -Wnon-virtual-dtor -Wold-style-cast
                -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Wformat=2
        )
        if(ENGINE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} INTERFACE -Werror)
        endif()
    endif()
endfunction()
