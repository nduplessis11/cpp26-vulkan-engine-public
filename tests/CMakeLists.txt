add_executable(engine_scaffold_smoke smoke/scaffold_smoke.cpp)
target_link_libraries(engine_scaffold_smoke PRIVATE engine::runtime)
add_test(NAME engine.scaffold.smoke COMMAND engine_scaffold_smoke)

# engine::runtime is an INTERFACE target that already carries engine::project_options
# and engine::project_warnings (see engine/CMakeLists.txt), so linking it alone brings
# the standard, warning, contract, and stdc++exp-for-std::print flags with it.
add_executable(engine_version_test unit/version_test.cpp)
target_link_libraries(engine_version_test PRIVATE engine::runtime)
add_test(NAME engine.unit.version COMMAND engine_version_test)

# Death test: version_component called out of contract, so a live assert aborts it.
# No test property is involved - neither WILL_FAIL nor PASS_REGULAR_EXPRESSION can
# rescue a SIGABRT, so the binary traps the abort and exits 0 itself. See the comment
# at the top of the source for why. This stays a plain exit-code test.
add_executable(engine_version_contract_test unit/version_component_contract_test.cpp)
target_link_libraries(engine_version_contract_test PRIVATE engine::runtime)
add_test(NAME engine.unit.version_component_contract COMMAND engine_version_contract_test)
