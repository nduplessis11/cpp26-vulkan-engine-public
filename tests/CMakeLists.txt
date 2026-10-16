add_executable(engine_scaffold_smoke smoke/scaffold_smoke.cpp)
target_link_libraries(engine_scaffold_smoke PRIVATE engine::runtime)
add_test(NAME engine.scaffold.smoke COMMAND engine_scaffold_smoke)

add_test(NAME engine.sandbox.exits COMMAND engine_sandbox)
set_tests_properties(engine.sandbox.exits PROPERTIES TIMEOUT 30)

# engine_add_gtest() supplies main(), engine build flags, and one CTest entry
# per TEST(). It also handles MinGW test registration; see cmake/TestHelpers.cmake.
# EXPECT_DEATH checks contract violations in separate processes within the same test binary.
# See docs/adr/0009-googletest-for-cpu-tests.md for the GoogleTest decision.
engine_add_gtest(engine_version_test core/version_test.cpp)
engine_add_gtest(engine_fake_handle_test core/fake_handle_test.cpp)
engine_add_gtest(engine_logger_test core/logger_test.cpp)
engine_add_gtest(engine_frame_clock_test core/frame_clock_test.cpp)
engine_add_gtest(engine_window_test platform/window_test.cpp)

if(ENGINE_BUILD_WAYLAND)
    engine_add_gtest(engine_wayland_test platform/wayland_test.cpp)
    # Backend headers are private to the engine, so only tests reach engine/src.
    target_include_directories(
        engine_wayland_test
        PRIVATE "${PROJECT_SOURCE_DIR}/engine/src"
    )
    target_link_libraries(engine_wayland_test PRIVATE engine_platform_wayland)
endif()
