cmake_minimum_required(VERSION 3.22)

project(DesktopHub
    VERSION 0.1.0
    DESCRIPTION "A customizable desktop dashboard for Windows."
    LANGUAGES CXX
)

# The MVP targets C++17 on purpose. Do not enable newer standards yet.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick)

# Sets up AUTOMOC/AUTORCC and other Qt defaults shared by all targets.
qt_standard_project_setup()

# Adopt the current Qt QML module layout rules explicitly so the build does
# not emit policy warnings.
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(DesktopHub
    src/main.cpp
)

# Registers the QML files as a module so they can be loaded by URI
# (see engine.loadFromModule in src/main.cpp).
qt_add_qml_module(DesktopHub
    URI DesktopHub
    VERSION 1.0
    QML_FILES
        src/gui/qml/Main.qml
)

# Keep the QML resource path stable regardless of the source directory layout.
set_source_files_properties(src/gui/qml/Main.qml
    PROPERTIES QT_RESOURCE_ALIAS Main.qml
)

# Build a GUI application without an extra console window.
set_target_properties(DesktopHub PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Match the warning level described in the project requirements.
if(MSVC)
    target_compile_options(DesktopHub PRIVATE /W4 /utf-8)
else()
    target_compile_options(DesktopHub PRIVATE -Wall -Wextra)
endif()

target_link_libraries(DesktopHub PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)
