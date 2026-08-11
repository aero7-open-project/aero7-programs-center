add_executable(programs-center-tests test_core.cpp)
target_link_libraries(programs-center-tests PRIVATE programs-center-core Qt6::Test)
add_test(NAME programs-center-core COMMAND programs-center-tests)
