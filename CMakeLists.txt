cmake_minimum_required(VERSION 3.24)
project(aero7-programs-center VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Widgets DBus Network Concurrent Test)
find_package(AppStreamQt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED IMPORTED_TARGET libalpm)
pkg_check_modules(APPSTREAM REQUIRED IMPORTED_TARGET appstream)

add_library(programs-center-core
  src/backend/PackageBackend.h
  src/backend/AlpmBackend.cpp
  src/backend/PackageTransaction.cpp
  src/metadata/Metadata.cpp
  src/models/Package.cpp
  src/sources/SourcePolicy.cpp
  src/sources/SourceCatalog.cpp
  src/DeepLink.cpp
)
target_include_directories(programs-center-core PUBLIC src)
target_link_libraries(programs-center-core PUBLIC Qt6::Core Qt6::Network Qt6::Concurrent AppStreamQt PkgConfig::ALPM PkgConfig::APPSTREAM)

add_executable(aero7-programs-center
  src/main.cpp
  src/ui/MainWindow.cpp
  src/ui/ProgramCard.cpp
  src/ui/SourceBadge.cpp
  resources/resources.qrc
)
file(GLOB_RECURSE AERO7_PACK_ICON_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/pack/*.png")
qt_add_resources(aero7-programs-center "aero7-programs-pack-icons"
  PREFIX "/aero7/pack"
  BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/pack"
  FILES ${AERO7_PACK_ICON_FILES})
target_link_libraries(aero7-programs-center PRIVATE programs-center-core Qt6::Widgets Qt6::DBus)

install(TARGETS aero7-programs-center RUNTIME DESTINATION bin)
install(FILES packaging/aero7-programs-center.desktop DESTINATION share/applications)
foreach(size 16 22 24 32 48 64 128 256 512)
  install(FILES resources/icons/${size}x${size}/aero7-programs-center.png DESTINATION share/icons/hicolor/${size}x${size}/apps)
endforeach()
install(FILES catalog/featured.json catalog/sources.json DESTINATION share/aero7-programs-center/catalog)
install(FILES ASSET-PROVENANCE.md
  resources/icons/pack/UPSTREAM-LICENSE
  resources/icons/pack/UPSTREAM-README.md
  DESTINATION share/doc/aero7-programs-center)
install(PROGRAMS helper/aero7-programs-center-helper DESTINATION lib/aero7-programs-center)
install(PROGRAMS helper/aero7-programs-center-trusted-installer
  helper/aero7-programs-center-permissions DESTINATION lib/aero7-programs-center)
install(FILES helper/org.aero7.programscenter.policy DESTINATION share/polkit-1/actions)

enable_testing()
add_subdirectory(tests)
