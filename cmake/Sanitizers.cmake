include_guard(GLOBAL)

function(engine_enable_sanitizers target_name)
    if(NOT ENGINE_ENABLE_SANITIZERS)
        return()
    endif()

    target_compile_options(${target_name} INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target_name} INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
endfunction()
