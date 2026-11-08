#include "services/Logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

#include "core/AppPaths.h"

namespace {
constexpr qint64 maximumLogSize = 1024 * 1024; // 1 MB

QMutex logMutex;
QString logFilePath;

// Keeps the current file plus two rotated generations (three files total).
void rotateIfNeeded()
{
    const QFileInfo info(logFilePath);
    if (!info.exists() || info.size() < maximumLogSize) {
        return;
    }

    const QString second = logFilePath + QStringLiteral(".2");
    const QString first = logFilePath + QStringLiteral(".1");

    QFile::remove(second);
    QFile::rename(first, second);
    QFile::rename(logFilePath, first);
}
} // namespace

void Logger::initialize()
{
    QMutexLocker locker(&logMutex);
    logFilePath = AppPaths::logFilePath();

    const QString directory = QFileInfo(logFilePath).absolutePath();
    QDir().mkpath(directory);
}

void Logger::info(const QString &message)
{
    write("INFO", message);
}

void Logger::warning(const QString &message)
{
    write("WARN", message);
}

void Logger::error(const QString &message)
{
    write("ERROR", message);
}

void Logger::write(const char *level, const QString &message)
{
    QMutexLocker locker(&logMutex);
    if (logFilePath.isEmpty()) {
        return;
    }

    rotateIfNeeded();

    QFile file(logFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    // One entry per line makes the log easy to scan.
    QString singleLine = message;
    singleLine.replace(QLatin1Char('\n'), QStringLiteral(" | "));

    const QString line = QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
        + QLatin1String(" [") + QLatin1String(level) + QLatin1String("] ")
        + singleLine + QLatin1Char('\n');

    file.write(line.toUtf8());
    file.flush();
}
