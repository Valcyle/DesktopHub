#pragma once

#include <QString>

// Minimal local file logger. It never sends data anywhere and must not be
// used for passwords, API keys, or other secrets.
class Logger
{
public:
    static void initialize();

    static void info(const QString &message);
    static void warning(const QString &message);
    static void error(const QString &message);

private:
    static void write(const char *level, const QString &message);
};
