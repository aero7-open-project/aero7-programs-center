add_executable(programs-center-tests test_core.cpp)
target_link_libraries(programs-center-tests PRIVATE programs-center-core Qt6::Test)
add_test(NAME programs-center-core COMMAND programs-center-tests)

add_executable(programs-center-offline-backend test_offline_backend.cpp)
target_link_libraries(programs-center-offline-backend PRIVATE programs-center-core Qt6::Test)
add_test(NAME programs-center-offline-backend COMMAND programs-center-offline-backend)

add_executable(programs-center-icon-independence test_icon_independence.cpp)
qt_add_resources(programs-center-icon-independence "aero7-programs-pack-icons-test"
  PREFIX "/aero7/pack"
  BASE "${CMAKE_SOURCE_DIR}/resources/icons/pack"
  FILES ${AERO7_PACK_ICON_FILES})
target_include_directories(programs-center-icon-independence PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(programs-center-icon-independence PRIVATE Qt6::Widgets)
add_test(NAME programs-center-icon-independence COMMAND programs-center-icon-independence)
set_tests_properties(programs-center-icon-independence PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(programs-center-home-layout test_home_layout.cpp
  ../src/ui/MainWindow.cpp ../src/ui/ProgramCard.cpp ../src/ui/SourceBadge.cpp ../resources/resources.qrc)
qt_add_resources(programs-center-home-layout "aero7-home-layout-pack-icons"
  PREFIX "/aero7/pack" BASE "${CMAKE_SOURCE_DIR}/resources/icons/pack"
  FILES ${AERO7_PACK_ICON_FILES})
target_link_libraries(programs-center-home-layout PRIVATE programs-center-core Qt6::Widgets Qt6::DBus Qt6::Test)
add_test(NAME programs-center-home-layout COMMAND programs-center-home-layout)
set_tests_properties(programs-center-home-layout PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_test(NAME programs-center-icon-policy
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_icon_policy.py)
