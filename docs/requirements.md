# Requirements

This document is the tracked description of the Desktop Hub MVP. It is derived from an internal Japanese requirements memo (version 0.2) and is the reference for implementation.

## 1. Product overview

Desktop Hub is a Windows desktop application that manages applications, folders, and websites in one place. It is a customizable dashboard rather than a plain launcher, and it is designed to grow into panels, widgets, and external service integrations.

Tagline: **Desktop Hub — One place for everything.**

## 2. Target users

- People who want to organize their Windows working environment
- People who need fast access to frequently used applications and folders
- People who separate environments by purpose (development, study, work, gaming)
- People who want to customize a dashboard with widgets

## 3. Platform and technology

| Area | Decision |
| --- | --- |
| Target OS | Windows 11 first; Windows 10 support is undecided |
| Language | C++17 (no C++20 features in the MVP) |
| UI | Qt 6 with Qt Quick / QML |
| Build system | CMake 3.22+ with Ninja |
| Storage | JSON, UTF-8 |
| Runtime UI languages | English and Japanese |

Technical constraints:

- Use Qt 6.5 LTS or newer; the development environment uses Qt 6.12.0.
- Keep the source encoding UTF-8 without BOM. MSVC builds must pass `/utf-8`.
- Keep build settings in `CMakePresets.json`.
- Warning level: `/W4` for MSVC, `-Wall -Wextra` for GCC/Clang.

## 4. User interface

- Modern design that fits Windows 11, inspired by Fluent Design.
- Dark theme, fixed in the MVP.
- Card and panel based layout.
- Minimum window size: 900 x 600. The number of card columns adapts to the window width.
- Sidebar pages: Home, Apps, Settings.
- Header: page title, search field (Apps), add button.

Page responsibilities:

- **Home**: shows a Favorites section first, followed by category sections.
- **Apps**: full item list, search, category filter, editing, and deletion.
- **Settings**: language, theme (display only in the MVP), category management, and the data storage location.

### Internationalization

- All user-facing text must go through `qsTr()`; never hard-code UI strings in QML.
- Source strings are written in English and translated into Japanese with Qt Linguist (`.ts` files, `lupdate` / `lrelease`).
- The default language follows the OS display language: Japanese on Japanese systems, English otherwise.
- The language can be changed in Settings. Changes apply immediately; a restart is an acceptable fallback.

### Keyboard operation

- `Ctrl+F`: focus the search field.
- `Esc`: clear the search field / cancel a dialog.
- `Enter`: launch the selected item / confirm a dialog.
- Double-click: launch an item.
- All primary operations must be possible with the keyboard alone.

## 5. MVP features

### 5.1 Item management

Supported item types:

- Windows executable (`.exe`)
- Windows shortcut (`.lnk`)
- Folder
- Website URL (`http` or `https`)

Supported operations: add, edit (name, target, category, favorite), delete, launch, open folder, and open URL in the default browser.

Deleting an item removes only the registration. The original file or folder is never deleted. The confirmation text must state this explicitly.

Registration flow:

1. Press the add button in the header.
2. Choose the type: application, folder, or URL.
3. Pick a file or folder in the file dialog, or enter a URL.
4. The name is filled in automatically from the file name without extension, the folder name, or the URL host.
5. The user may change the name, category, and favorite flag.
6. Confirming adds the item to the list and saves immediately.

Validation:

- Names are trimmed and must be 1 to 100 characters.
- Paths must be absolute and must exist at registration time.
- URLs accept only `http` and `https`. If the scheme is omitted, prepend `https://`.
- If the same path or URL is already registered, show a warning but allow the user to continue.
- Duplicate names are allowed.
- Executable arguments and working directories are out of scope for the MVP.

Launching:

- Local items use `QDesktopServices::openUrl(QUrl::fromLocalFile(...))`.
- URLs use `QDesktopServices::openUrl(QUrl(...))`.
- A `false` return value is treated as a launch failure.
- Elevation and post-launch crash detection are out of scope.

### 5.2 Search

- Search by item name and category name.
- Case-insensitive partial match.
- Filtering updates as the user types.
- Do not update filtering while a Japanese IME composition is in progress; use the committed string.
- Show "No matching items" when nothing matches.
- Clearing the field restores the full list.
- Search applies to the currently visible list (Home or Apps).

### 5.3 Categories

- Items can be assigned to one category.
- Categories are stored as an independent list with IDs and names.
- Items without a category are shown as "Uncategorized".
- Category names are trimmed, 1 to 50 characters, and must be unique.
- Categories are added, renamed, and deleted in Settings.
- Deleting a category moves its items back to "Uncategorized".
- Nested categories are out of scope.

### 5.4 Favorites

- Items can be marked as favorites or unmarked.
- Home shows a Favorites section first, then category sections.
- Sort order is ascending creation time. Manual reordering is out of scope.
- Remove a favorite immediately removes it from the Home favorites section.
- Favorite items also remain visible in their category (or uncategorized) section.

### 5.5 Data storage

- Store items and settings as UTF-8 JSON.
- Restore data after a restart; start with an empty state on first launch.
- Save automatically after every change (add, edit, delete, favorite, category, settings).
- Write atomically with `QSaveFile`; keep one backup generation as `data.json.bak`.
- Store `data.json` under `QStandardPaths::AppDataLocation`.
- Store logs under `QStandardPaths::AppLocalDataLocation/logs/`.
- Keep path construction in an `AppPaths` class.
- Use a temporary application identifier until the final name is decided, and provide a migration path when it changes.

### 5.6 Error handling

| Situation | Behavior |
| --- | --- |
| File or folder missing | Check at launch, show an error, keep the item. The user can edit it. |
| Invalid URL | Validate at registration, show the reason inline, and do not save. |
| Launch failure | Show a dialog with the path and item type. |
| Corrupt data file | Rename it to `data.corrupt-YYYYMMDD-HHMMSS.json`, start with empty data, and notify the user of the backup path. If the rename fails, start read-only and refuse to overwrite. |
| Save failure | Show a dialog, keep changes in memory, and offer a retry. Warn before exit while unsaved. |

- Blocking errors use modal dialogs.
- Input errors are shown inline, not in dialogs.
- Logs are local only, contain timestamp/level/message, rotate at 1 MB, and keep at most three generations.
- Never log passwords, API keys, or tokens.

### 5.7 Icons

- Extract real icons for executables, shortcuts, and folders through the Windows Shell API (`SHGetFileInfoW` or `IShellItemImageFactory`).
- Provide 32 px and 48 px variants and choose the appropriate size for the display scale.
- Cache extracted icons in memory.
- Fall back to a type-specific icon when extraction fails.
- Use a fixed web icon for URLs.
- Do not block the UI thread for long during extraction.
- No disk icon cache in the MVP.

### 5.8 Single instance

- Allow only one running instance.
- Use `QLockFile`; when a second instance starts, bring the existing window to the front through `QLocalServer` / `QLocalSocket`.
- If the existing instance cannot be reached, inform the user and exit.
- This prevents concurrent JSON writes.

## 6. Data model (schemaVersion 1)

```json
{
  "schemaVersion": 1,
  "items": [
    {
      "id": "0b6f5f0e-4f2a-4c1e-9f0a-2b7c9d1e3a45",
      "name": "Visual Studio Code",
      "type": "exe",
      "target": "C:\\Users\\user\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe",
      "categoryId": "dev",
      "favorite": true,
      "createdAt": "2026-10-31T12:00:00Z"
    }
  ],
  "categories": [
    { "id": "dev", "name": "Development" }
  ],
  "settings": {
    "language": "en",
    "theme": "dark"
  }
}
```

### Items

| Field | Type | Required | Notes |
| --- | --- | --- | --- |
| `id` | string (UUID v4) | yes | Unique item ID |
| `name` | string | yes | Display name, 1-100 characters |
| `type` | string | yes | `exe`, `lnk`, `folder`, or `url` |
| `target` | string | yes | Absolute path or URL |
| `categoryId` | string or null | no | `null` means uncategorized |
| `favorite` | boolean | yes | Favorite flag |
| `createdAt` | string (ISO 8601) | yes | Creation time in UTC |

### Categories

| Field | Type | Required | Notes |
| --- | --- | --- | --- |
| `id` | string (UUID v4) | yes | Unique category ID |
| `name` | string | yes | 1-50 characters, unique |

### Settings

| Field | Type | Required | Notes |
| --- | --- | --- | --- |
| `language` | string | yes | `en` or `ja` |
| `theme` | string | yes | Fixed to `dark` in the MVP |

### Load rules

- If `schemaVersion` is greater than 1, refuse to start and show an error.
- If `schemaVersion` is less than 1, it will be handled by a future migration (not produced by v1).
- Skip items with an unknown `type`, and log the problem.
- Treat items that reference a missing `categoryId` as uncategorized.
- Skip items with missing required fields, log them, and tell the user how many were skipped or adjusted.

## 7. Out of scope for the MVP

Desktop widgets, floating panels, advanced shortcuts, workspaces, batch launching, window arrangement, clipboard management, system information, calendar/Discord/GitHub integrations, cloud sync, accounts, online licensing, payments, plugin installation, drag-and-drop registration, manual reordering, recently used items, import/export, and executable arguments.

## 8. Non-functional requirements

### Performance

- Cold start within 3 seconds on an SSD with 1,000 items.
- Search and filtering remain responsive with 1,000 items (target: under 100 ms).
- Resident memory target: under 200 MB.
- Icon extraction must not block rendering.

### Display

- Minimum window size 900 x 600.
- Usable at 1280 x 720 and above.
- No visual breakage at 100%-200% display scaling.
- Column count adapts when resizing.

### Reliability

- Atomic saves so a power loss cannot leave half-written JSON.
- Corrupt data is backed up, never deleted.
- Single-instance enforcement prevents concurrent writes.
- Launch failures and missing paths do not stop the rest of the application.

### Security and privacy

- Fully offline in the MVP.
- No administrator privileges required.
- No secrets in logs.
- No external communication in the MVP.

### Accessibility

- Primary operations complete with the keyboard.
- Focus is always visible.
- Contrast is at least as good as the Qt Quick Controls defaults.
- State is never conveyed by color alone.

### Maintainability

- UI strings live in translation files.
- The storage format is versioned with `schemaVersion`.
- Dependency licenses are documented.
- Important logic has beginner-friendly comments.
