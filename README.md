# Desktop Hub

**One place for everything.**

Desktop Hub is a Windows desktop dashboard that keeps the applications, folders, and websites you use every day in one place. Instead of being just another launcher, it is built around a customizable dashboard that can grow with future panels, widgets, and integrations.

> Status: Phase 4 is complete. The application now includes logging, single-instance protection, read-only mode, and unsaved-change warnings. The remaining work is MVP polish and release preparation.

## Features

- Dark dashboard window with a fixed sidebar (Home, Apps, Settings)
- English and Japanese UI, switchable from Settings without restarting
- Register applications (`.exe`), shortcuts (`.lnk`), folders, and `http`/`https` URLs
- Real Windows Shell icons with a built-in fallback for missing icons
- Double-click or press Enter to launch an item
- Inline validation for names, paths, and URLs
- Case-insensitive search by item name and category name, shared by Home and Apps
- Category filter on Apps; add, rename, and delete categories in Settings
- Favorites section on Home, followed by category sections and uncategorized items
- Automatic JSON saving after every change, with a one-generation backup
- Missing-file detection at startup and at launch time
- Read-only mode when the data file cannot be prepared, with editing disabled
- Warning before quitting when changes could not be saved
- Single-instance enforcement that raises the existing window
- In-app updates: check GitHub Releases, verify SHA-256, replace the portable build, and restart
- Local logging with 1 MB rotation and three generations
- Recovery flow when the data file is corrupt
- Responsive layout that works down to the 900 x 600 minimum size

Planned: final UI polish, accessibility review, release packaging polish, code signing, and license selection.

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

## Packaging

Builds a portable folder, ZIP, and checksum:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

Outputs:

- `dist/DesktopHub-<version>-win64/` — the portable application folder
- `dist/DesktopHub-<version>-win64.zip` — the release asset
- `dist/SHA256SUMS.txt` — the checksum consumed by the updater

The script runs `windeployqt`, bundles `DesktopHubUpdater.exe`, and places the
Qt LGPL notices under `LICENSES/Qt/`. `dist/` is not committed.

## Data storage

- Data file: `data.json` under `QStandardPaths::AppDataLocation`
- Previous version: `data.json.bak`
- Corrupt files are moved to `data.corrupt-YYYYMMDD-HHMMSS.json`
- Format: UTF-8 JSON with `schemaVersion: 1`
- The file is written with `QSaveFile` for atomic replacement
- Logs: `logs/desktophub.log` under `QStandardPaths::AppLocalDataLocation`
- Log rotation: at 1 MB, keeping three generations in total
- Only one instance can run; a second launch raises the existing window

## Updates

- The update feed is the latest GitHub Release:
  `https://api.github.com/repos/Valcyle/DesktopHub/releases/latest`
- Publish `DesktopHub-<version>-win64.zip` and `SHA256SUMS.txt` as release assets.
- Startup checks are enabled by default and can be disabled in Settings. Manual
  checks are available at any time.
- Installation is: download → SHA-256 verification → extract with Windows
  `tar.exe` → `DesktopHubUpdater.exe` waits for the app to exit → files are
  replaced → the app restarts.
- The portable folder must be writable by the current user.
- Builds are currently unsigned. Windows SmartScreen may warn on first launch.
  The checksum protects against corrupt downloads, not against a compromised
  GitHub account; code signing is the next step for release hardening.
- The app sends no personal data. GitHub receives the HTTP request metadata
  (IP address and user agent) only when a check runs.

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
├── scripts/         # package.ps1
├── src/
├── third_party/     # Qt LGPL notices
└── README.md
```

Automated tests are not set up yet; the current verification is manual.

## Documentation

- [Requirements](docs/requirements.md) — MVP scope and behavior
- [Development environment](docs/dev-environment.md) — toolchain versions and build decisions
- [Third-party licenses](docs/third-party-licenses.md) — dependencies and license obligations
- [Qt license notices](third_party/qt/NOTICE.md) — Qt-only LGPLv3 notices
- [Manual test checklist](docs/testing.md) — release verification steps

## Privacy

Desktop Hub works fully offline. It does not send telemetry or usage data.

## License

Desktop Hub's own source-code license has not been chosen yet. The license and
distribution model will be decided before the first public release. The product
name and logo are separate from the source code license.

Qt is used under the LGPLv3. Its license texts and notices are stored in
[`third_party/qt/`](third_party/qt/NOTICE.md) and copied into release packages
under `LICENSES/Qt/`. Those files apply to Qt only; they do not set the license
of Desktop Hub.
