# Wayland lessons: generate xdg-shell client code during the build. The XML
# comes from third_party/wayland-protocols, so the generated code does not
# depend on the installed wayland-protocols package. Expose the generated
# code only to the Wayland backend.

include_guard(GLOBAL)

function(engine_add_xdg_shell_protocol target_name)
    find_package(PkgConfig REQUIRED)
    pkg_get_variable(wayland_scanner wayland-scanner wayland_scanner)
    # Generation runs during the build, so check for the scanner here to
    # fail during configuration instead of partway through a build.
    if(NOT wayland_scanner)
        message(FATAL_ERROR "pkg-config did not find wayland-scanner")
    endif()

    set(protocols_dir "${PROJECT_SOURCE_DIR}/third_party/wayland-protocols")
    set(xml "${protocols_dir}/stable/xdg-shell/xdg-shell.xml")
    set(out_dir "${CMAKE_CURRENT_BINARY_DIR}/protocols")
    set(header "${out_dir}/xdg-shell-client-protocol.h")
    set(code "${out_dir}/xdg-shell-protocol.c")
    file(MAKE_DIRECTORY "${out_dir}")

    # The build tool reruns the scanner only when an output is missing or
    # older than the XML or the scanner executable.
    add_custom_command(
        OUTPUT "${header}" "${code}"
        COMMAND "${wayland_scanner}" client-header "${xml}" "${header}"
        COMMAND "${wayland_scanner}" private-code "${xml}" "${code}"
        DEPENDS "${xml}" "${wayland_scanner}"
        COMMENT "Generating xdg-shell client protocol code"
        VERBATIM
    )
    # make lint builds this target to create the header without compiling.
    # The object library depends on it so the Makefile generator does not
    # run the command from two targets at once.
    add_custom_target(${target_name}_generate DEPENDS "${header}" "${code}")

    add_library(${target_name} OBJECT "${code}")
    add_dependencies(${target_name} ${target_name}_generate)
    # The PUBLIC file set marks the header as part of this target's
    # interface. CMake then makes sources of targets that link this one
    # compile after the header exists, and adds out_dir to their include
    # path.
    target_sources(
        ${target_name}
        PUBLIC FILE_SET HEADERS BASE_DIRS "${out_dir}" FILES "${header}"
    )
    target_link_libraries(${target_name} PUBLIC PkgConfig::WaylandClient)
endfunction()
