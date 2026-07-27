add_executable(engine_scaffold_smoke smoke/scaffold_smoke.cpp)
target_link_libraries(engine_scaffold_smoke PRIVATE engine::runtime)
add_test(NAME engine.scaffold.smoke COMMAND engine_scaffold_smoke)
