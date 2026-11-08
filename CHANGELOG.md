# Changelog

All notable changes to Desktop Hub are documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project uses semantic versioning.

## [0.1.2] - 2026-11-08

### Fixed

- Update downloads now use the GitHub API asset endpoint with the correct
  `Accept` header. The public release download endpoint returned HTTP 504
  during the beta, so the updater needed a more reliable route.
- The feed parser accepts both the release list (GitHub API) and a single
  release object (local test feeds), and selects the highest version.

## [0.1.1] - 2026-11-08

First beta release.

### Added

- Item management for applications, shortcuts, folders, and URLs
- Windows Shell icons with fallbacks for missing or unreadable icons
- Case-insensitive search by item and category name
- Categories with add, rename, delete, and filtering
- Favorites and the Home dashboard sections
- JSON storage with atomic writes, one backup generation, and corruption recovery
- English and Japanese UI with an in-app language switch
- Missing-target notices at startup and launch time
- Read-only mode and unsaved-change warnings
- Single-instance protection that raises the existing window
- Local logging with 1 MB rotation
- In-app updates with SHA-256 verification, replacement, and restart
- Portable Windows x64 packaging with Qt LGPL notices

### Changed

- Finalized the product identity: Desktop Hub / DesktopHub / Valcyle
- Adopted the Apache License 2.0 and a separate trademark policy

### Known limitations

- The build is unsigned; Windows SmartScreen may warn.
- Windows 10 support is not confirmed.
- Automated tests are not set up yet.
