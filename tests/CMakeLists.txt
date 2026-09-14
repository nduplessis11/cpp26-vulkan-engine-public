add_executable(engine_scaffold_smoke smoke/scaffold_smoke.cpp)
target_link_libraries(engine_scaffold_smoke PRIVATE engine::runtime)
add_test(NAME engine.scaffold.smoke COMMAND engine_scaffold_smoke)

add_test(NAME engine.sandbox.exits COMMAND engine_sandbox)
set_tests_properties(engine.sandbox.exits PROPERTIES TIMEOUT 30)

# engine_add_gtest() supplies main(), engine build flags, and one CTest entry
# per TEST(). It also handles MinGW test registration; see cmake/TestHelpers.cmake.
# EXPECT_DEATH checks contract violations in separate processes within the same test binary.
# See docs/adr/0009-googletest-for-cpu-tests.md for the GoogleTest decision.
engine_add_gtest(engine_version_test unit/version_test.cpp)
engine_add_gtest(engine_fake_handle_test unit/fake_handle_test.cpp)
engine_add_gtest(engine_logger_test unit/logger_test.cpp)
engine_add_gtest(engine_frame_clock_test unit/frame_clock_test.cpp)
