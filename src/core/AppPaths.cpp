#include "core/AppPaths.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

namespace {
QString appDataDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
} // namespace

QString AppPaths::dataFilePath()
{
    return QDir(appDataDirectory()).filePath(QStringLiteral("data.json"));
}

QString AppPaths::backupFilePath()
{
    // One generation of backup, as required by the storage rules.
    return dataFilePath() + QStringLiteral(".bak");
}

QString AppPaths::corruptFilePath()
{
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    return QDir(appDataDirectory())
        .filePath(QStringLiteral("data.corrupt-%1.json").arg(stamp));
}

QString AppPaths::logsDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("logs"));
}

QString AppPaths::logFilePath()
{
    return QDir(logsDirectory()).filePath(QStringLiteral("desktophub.log"));
}

QString AppPaths::lockFilePath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("desktophub.lock"));
}
