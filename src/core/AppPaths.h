#pragma once

#include <QString>

// Central place for every path the application uses for user data.
//
// Product identity:
//   Display name: Desktop Hub
//   Internal ID:  DesktopHub
//   Organization: Valcyle
//
// On Windows this resolves to:
//   Data: %APPDATA%\Valcyle\DesktopHub
//   Logs: %LOCALAPPDATA%\Valcyle\DesktopHub\logs
//
// Treat these identifiers as stable. Renaming them requires a migration.
class AppPaths
{
public:
    static QString dataFilePath();
    static QString backupFilePath();
    static QString corruptFilePath();
    static QString logsDirectory();
    static QString logFilePath();
    static QString lockFilePath();
};
