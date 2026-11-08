# Development environment

This document records the toolchain used for development. It intentionally contains version information only; do not add machine-specific absolute paths or user names here.

## Verified environment

| Component | Version / choice | Notes |
| --- | --- | --- |
| OS | Windows 11 Home (10.0.26200) | Windows 10 support is undecided |
| Qt | 6.12.0 (`mingw_64`) | Installed under `C:/Qt` |
| Compiler | MinGW-w64 GCC 13.1.0 | Bundled with Qt (`C:/Qt/Tools/mingw1310_64`) |
| CMake | 3.30.5 | Qt-bundled CMake; the system CMake is a pre-release and is not used |
| Ninja | 1.12.1 | Qt-bundled Ninja |
| Git | 2.53.0 | |
| Visual Studio | 2026 Community | Installed, but no Qt MSVC kit is installed |

## Toolchain decision

The first candidate in the requirements was MSVC 2022. This machine has a Qt MinGW kit only, and the installed Visual Studio is 2026 rather than 2022, so the Qt 6.12.0 `msvc2022_64` kit would be an unverified combination.

Phase 0 therefore uses the Qt-bundled **MinGW 13.1.0** toolchain:

- It is the pairing shipped and tested by the Qt installer.
- It requires no additional multi-gigabyte download.
- The application code is portable C++17/QML, so an MSVC preset can be added later without restructuring.

Do not mix this MinGW with a separately installed MinGW. Always use the compiler from `C:/Qt/Tools/mingw1310_64`.

## Qt version

Qt 6.12.0 is used instead of the 6.8 LTS series. For an LGPL/open-source project, staying on the current minor release gives access to open-source patch releases; LTS patch releases are primarily aimed at commercial customers.

The minimum supported Qt version remains 6.5, as declared in `CMakeLists.txt`.

## Build configuration

- Generator: Ninja
- Build type: Debug for development, Release for packaging tests
- Presets: `mingw-debug`, `mingw-release` in `CMakePresets.json`
- C++ standard: C++17
- Warnings: `-Wall -Wextra` (MinGW)

## Verification

Phase 0 is complete when the following commands configure, build, and start a Qt Quick window:

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```

## Revisit before release

- Confirm the minimum Windows version (Windows 10 support).
- Decide whether to ship MinGW or add an MSVC build.
- Verify deployment with `windeployqt`, including the MinGW runtime DLLs.
- Re-check the Qt version and its license obligations (see `docs/third-party-licenses.md`).
