# Desktop Hub

**One place for everything.**

Desktop Hub is a Windows desktop dashboard that keeps the applications, folders, and websites you use every day in one place. Instead of being just another launcher, it is built around a customizable dashboard that can grow with future panels, widgets, and integrations.

> Status: Phase 2 is complete. Items can be registered, edited, deleted, launched, and saved to disk. Search, categories, and favorites are next.

## Features

- Dark dashboard window with a fixed sidebar (Home, Apps, Settings)
- English and Japanese UI, switchable from Settings without restarting
- Register applications (`.exe`), shortcuts (`.lnk`), folders, and `http`/`https` URLs
- Real Windows Shell icons with a built-in fallback for missing icons
- Double-click or press Enter to launch an item
- Inline validation for names, paths, and URLs
- Automatic JSON saving after every change, with a one-generation backup
- Recovery flow when the data file is corrupt
- Responsive layout that works down to the 900 x 600 minimum size

Planned: search, categories, favorites on Home, logging, single-instance enforcement, and the final UI polish.

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

## Data storage

- Data file: `data.json` under `QStandardPaths::AppDataLocation`
- Previous version: `data.json.bak`
- Corrupt files are moved to `data.corrupt-YYYYMMDD-HHMMSS.json`
- Format: UTF-8 JSON with `schemaVersion: 1`
- The file is written with `QSaveFile` for atomic replacement

Application data and logs can contain local file paths, including your Windows user name, so review them before sharing. Runtime data, backups, and logs are excluded from this repository by `.gitignore`.

## Translations

English is the source language. Every user-facing string goes through `qsTr()` or `tr()`, and Japanese translations live in `i18n/desktophub_ja.ts`.

After adding or changing strings, refresh the translation files:

```powershell
cmake --build --preset mingw-debug --target update_translations
```

The `.qm` files are generated at build time and embedded in the executable. The initial language follows the OS display language or the language stored in `data.json`; it can be changed in Settings.

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
│   ├── core/        # paths, settings, shared data types
│   ├── gui/         # QML-facing controller and UI
│   ├── models/      # item list model
│   └── services/    # JSON storage and shell icon provider
└── README.md
```

Automated tests are not set up yet; the current verification is manual.

## Documentation

- [Requirements](docs/requirements.md) — MVP scope and behavior
- [Development environment](docs/dev-environment.md) — toolchain versions and build decisions
- [Third-party licenses](docs/third-party-licenses.md) — dependencies and license obligations

## Privacy

Desktop Hub works fully offline. It does not send telemetry or usage data.

## License

A license has not been chosen yet. The license and distribution model will be decided before the first public release. The product name and logo are separate from the source code license.
