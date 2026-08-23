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
                # Not in -Wall or -Wextra, because it fires on legitimate pre-C++11 code.
                # Declaring a destructor suppresses the implicit move operations but keeps
                # the implicit copies, so std::move on an owning type silently copies it.
                -Wdeprecated-copy-dtor
        )
        if(ENGINE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} INTERFACE -Werror)
        endif()
    endif()
endfunction()
