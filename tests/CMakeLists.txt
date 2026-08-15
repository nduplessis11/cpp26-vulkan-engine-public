add_executable(engine_scaffold_smoke smoke/scaffold_smoke.cpp)
target_link_libraries(engine_scaffold_smoke PRIVATE engine::runtime)
add_test(NAME engine.scaffold.smoke COMMAND engine_scaffold_smoke)

# engine_add_gtest (cmake/TestHelpers.cmake) links engine::runtime and
# GTest::gtest_main, then registers one CTest entry per TEST() case. It also
# picks a registration strategy that survives the MinGW cross build; the reason
# is documented there.
#
# engine::runtime is an INTERFACE target that already carries
# engine::project_options and engine::project_warnings (see
# engine/CMakeLists.txt), so linking it alone brings the standard, warning,
# contract, and stdc++exp-for-std::print flags with it.
#
# The out-of-contract death tests live in this same binary now. They previously
# needed a separate executable because a SIGABRT is a CTest result category that
# neither WILL_FAIL nor PASS_REGULAR_EXPRESSION inverts, and the workaround for
# that made everything after the abort unreachable. EXPECT_DEATH forks a
# subprocess per assertion, so unit/version_component_contract_test.cpp is gone.
# See docs/adr/0009-googletest-for-cpu-tests.md.
engine_add_gtest(engine_version_test unit/version_test.cpp)
engine_add_gtest(engine_fake_handle_test unit/fake_handle_test.cpp)
