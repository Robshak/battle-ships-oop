cmake_minimum_required(VERSION 3.21)
project(BattleShips VERSION 1.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets)
qt_add_executable(BattleShips WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/app/Application.cpp
    src/app/Session.cpp
    src/model/Ship.cpp
    src/model/Field.cpp
    src/game/Strategy.cpp
    src/game/Game.cpp
    src/ui/Widgets.cpp
    src/ui/Window.cpp
)
target_include_directories(BattleShips PRIVATE src)
target_link_libraries(BattleShips PRIVATE Qt6::Widgets)
if(WIN32)
    target_link_libraries(BattleShips PRIVATE advapi32)
endif()
set_target_properties(BattleShips PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER ru.robshak.battleships
    MACOSX_BUNDLE_BUNDLE_NAME "Battle Ships"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)
if(MSVC)
    target_compile_options(BattleShips PRIVATE /W4 /utf-8)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
else()
    target_compile_options(BattleShips PRIVATE -Wall -Wextra -Wpedantic)
endif()
install(TARGETS BattleShips BUNDLE DESTINATION . RUNTIME DESTINATION .)
install(FILES THIRD_PARTY.md DESTINATION .)
install(DIRECTORY licenses/ DESTINATION licenses)
