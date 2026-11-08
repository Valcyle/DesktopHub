# Desktop Hub

**One place for everything.**

Desktop Hub is a Windows desktop dashboard that keeps the applications, folders, and websites you use every day in one place. Instead of being just another launcher, it is built around a customizable dashboard that can grow with future panels, widgets, and integrations.

> Status: early development. Phase 1 (basic window, English/Japanese translation base) is complete. Item management and storage are next.

## Features

- Dark dashboard window with a fixed sidebar (Home, Apps, Settings)
- English and Japanese UI, switchable from Settings without restarting
- Responsive layout that works down to the 900 x 600 minimum size

Planned for the next phases: item registration (apps, folders, URLs), search, categories, favorites, JSON persistence, Shell icons, and error handling.

## Requirements

- Windows 11 (Windows 10 support is still undecided)
- Qt 6.5 or newer — developed and tested with **Qt 6.12.0 (MinGW 13.1.0)**
- CMake 3.22 or newer
- Ninja
- A C++17 compiler

## Build

The checked-in `CMakePresets.json` points at the Qt installation used during development. Adjust the paths in `mingw-base` if your Qt installation lives elsewhere.

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```

To run the application from a terminal, make the Qt and MinGW runtime directories available:

```powershell
$env:PATH = "C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
.\build\mingw-debug\DesktopHub.exe
```

You can also open the project folder directly in Qt Creator or in VS Code with the CMake Tools extension.

A release build is available through the `mingw-release` preset.

## Translations

English is the source language. Every user-facing string goes through `qsTr()`, and Japanese translations live in `i18n/desktophub_ja.ts`.

After adding or changing strings, refresh the translation files:

```powershell
cmake --build --preset mingw-debug --target update_translations
```

The `.qm` files are generated at build time and embedded in the executable. The initial language follows the OS display language; it can be changed in Settings.

## Project layout

```text
DesktopHub/
├── CMakeLists.txt
├── CMakePresets.json
├── docs/
│   ├── dev-environment.md
│   ├── requirements.md
│   └── third-party-licenses.md
├── i18n/
│   ├── desktophub_en.ts
│   └── desktophub_ja.ts
├── src/
│   ├── main.cpp
│   ├── core/
│   │   └── AppSettings.h/.cpp
│   └── gui/
│       └── qml/
│           ├── Main.qml
│           ├── Sidebar.qml
│           ├── HomePage.qml
│           ├── AppsPage.qml
│           ├── SettingsPage.qml
│           └── Theme.qml
└── README.md
```

The structure will grow with `models/`, `services/`, and `tests/` as later phases are implemented.

## Documentation

- [Requirements](docs/requirements.md) — MVP scope and behavior
- [Development environment](docs/dev-environment.md) — toolchain versions and build decisions
- [Third-party licenses](docs/third-party-licenses.md) — dependencies and license obligations

## Privacy

Desktop Hub works fully offline. It does not send telemetry or usage data.

Application data and logs are stored locally under the paths provided by `QStandardPaths`. Logs can contain full file paths, including your Windows user name, so please review them before sharing them in a bug report. Runtime data, backups, and logs are excluded from this repository by `.gitignore`.

## License

A license has not been chosen yet. The license and distribution model will be decided before the first public release. The product name and logo are separate from the source code license.
