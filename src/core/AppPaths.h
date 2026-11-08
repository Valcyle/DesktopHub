#pragma once

#include <QString>

// Central place for every path the application uses for user data.
// Changing the application or organization name changes these paths, so keep
// the migration logic in mind when the final product name is chosen.
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
