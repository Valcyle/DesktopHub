# Manual test checklist

Automated tests are not set up yet. Run this checklist on Windows 11 with a
debug build and, for release checks, the packaged build.

## Startup and data

- [ ] First launch shows an empty Home page.
- [ ] A second launch raises the existing window and exits.
- [ ] Data survives a restart.
- [ ] Corrupt `data.json` is moved to `data.corrupt-YYYYMMDD-HHMMSS.json` and a notice appears.
- [ ] A missing item target produces a startup notice without deleting the item.
- [ ] A save failure shows a retry dialog and the quit warning appears on close.
- [ ] The log file is written and rotates at 1 MB.

## Items

- [ ] Add `.exe`, `.lnk`, folder, and URL items.
- [ ] Launch each type; missing local files show an error.
- [ ] Edit name, target, category, and favorite.
- [ ] Delete an item and confirm the original file still exists.
- [ ] Duplicate targets warn but can be registered.
- [ ] Invalid names, paths, and URLs show inline errors.

## Search and categories

- [ ] Search matches names and category names, case-insensitively.
- [ ] Japanese IME composition does not filter until the text is committed.
- [ ] Clearing the search restores the list.
- [ ] Category filter works, including "Uncategorized".
- [ ] Add, rename, and delete categories in Settings.
- [ ] Deleting a category moves its items to Uncategorized.
- [ ] Home shows Favorites, category sections, and Uncategorized in order.

## Updates

- [ ] Manual check reports "latest version" when the release is not newer.
- [ ] Automatic check can be disabled in Settings and the choice persists.
- [ ] An available update shows the release notes and the install button.
- [ ] Install downloads the ZIP, verifies SHA-256, extracts, replaces files, and restarts.
- [ ] A bad checksum is rejected and the app stays open.
- [ ] A missing `DesktopHubUpdater.exe` is reported instead of partially updating.

## Display and accessibility

- [ ] Window works at the 900 x 600 minimum size.
- [ ] Layout works at 100%, 150%, and 200% Windows scaling.
- [ ] All primary operations work with the keyboard.
- [ ] Focus is visible on sidebar, lists, buttons, and dialogs.
- [ ] Favorite and checked states are visible without relying on color alone.
- [ ] English and Japanese text is complete in every screen and dialog.
