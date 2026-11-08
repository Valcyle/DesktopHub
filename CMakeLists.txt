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

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    LinguistTools
)

# Sets up AUTOMOC/AUTORCC and other Qt defaults shared by all targets.
qt_standard_project_setup()

# Adopt the current Qt QML module layout rules explicitly so the build does
# not emit policy warnings.
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(DesktopHub
    src/main.cpp
    src/core/AppPaths.h
    src/core/AppPaths.cpp
    src/core/AppSettings.h
    src/core/AppSettings.cpp
    src/core/Category.h
    src/core/Item.h
    src/gui/AppController.h
    src/gui/AppController.cpp
    src/models/CategoryModel.h
    src/models/CategoryModel.cpp
    src/models/HomeModel.h
    src/models/HomeModel.cpp
    src/models/ItemFilterModel.h
    src/models/ItemFilterModel.cpp
    src/models/ItemModel.h
    src/models/ItemModel.cpp
    src/services/IconProvider.h
    src/services/IconProvider.cpp
    src/services/JsonStorage.h
    src/services/JsonStorage.cpp
    src/services/Logger.h
    src/services/Logger.cpp
    src/services/SingleInstance.h
    src/services/SingleInstance.cpp
    src/services/UpdateService.h
    src/services/UpdateService.cpp
)

# All QML files are registered at the root of the "DesktopHub" module.
# Keeping them flat also keeps the QML module layout rules simple.
set(DESKTOPHUB_QML_FILES
    src/gui/qml/Main.qml
    src/gui/qml/Theme.qml
    src/gui/qml/Sidebar.qml
    src/gui/qml/HomePage.qml
    src/gui/qml/AppsPage.qml
    src/gui/qml/SettingsPage.qml
    src/gui/qml/ItemDialog.qml
    src/gui/qml/CategoryDialog.qml
    src/gui/qml/HubSearchField.qml
    src/gui/qml/HubCheckBox.qml
)

foreach(qml_file IN LISTS DESKTOPHUB_QML_FILES)
    get_filename_component(qml_file_name "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_file_name}")
endforeach()

# Theme.qml declares "pragma Singleton" and must be registered accordingly.
set_source_files_properties(src/gui/qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(DesktopHub
    URI DesktopHub
    VERSION 1.0
    QML_FILES
        ${DESKTOPHUB_QML_FILES}
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

# "src" is used by our own includes; the subdirectories let the generated QML
# type registration find headers such as <AppSettings.h> and <AppController.h>.
target_include_directories(DesktopHub PRIVATE
    src
    src/core
    src/gui
    src/models
    src/services
)

target_link_libraries(DesktopHub PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# The application version is also used by the in-app updater.
target_compile_definitions(DesktopHub PRIVATE
    DESKTOPHUB_VERSION="${PROJECT_VERSION}"
)

# Standalone replacement helper for the portable update flow. It intentionally
# links no Qt libraries so it can run from a temporary directory.
if(WIN32)
    add_executable(DesktopHubUpdater
        src/updater/main.cpp
    )
    set_target_properties(DesktopHubUpdater PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_compile_features(DesktopHubUpdater PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(DesktopHubUpdater PRIVATE /W4 /utf-8)
    else()
        target_compile_options(DesktopHubUpdater PRIVATE -Wall -Wextra)
    endif()
    target_link_libraries(DesktopHubUpdater PRIVATE shell32)
    if(MINGW)
        # wWinMain is the entry point, and the updater must be a standalone
        # executable: it runs from a temporary folder while replacing the app.
        target_link_options(DesktopHubUpdater PRIVATE
            -municode
            -static
            -static-libgcc
            -static-libstdc++
        )
    endif()
endif()

if(WIN32)
    # Shell icon extraction and COM initialization.
    target_link_libraries(DesktopHub PRIVATE shell32 comctl32 ole32)
endif()

# Compile the translation files into the executable under ":/i18n".
# The update_translations target refreshes the .ts files with lupdate.
qt_add_translations(DesktopHub
    TS_FILES
        i18n/desktophub_en.ts
        i18n/desktophub_ja.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete -locations none
)
