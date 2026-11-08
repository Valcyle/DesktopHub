#pragma once

#include <QString>
#include <QVector>

#include "core/Item.h"

struct StorageLoadResult
{
    QVector<Item> items;
    QString language;          // Empty when the stored language is unknown.
    int skippedItems = 0;
    QString corruptBackupPath; // Set when a corrupt file was moved aside.
    QString warning;           // User-facing notice for non-fatal problems.
    QString fatalMessage;      // Set when the app must not save any changes.
    bool readOnly = false;
};

// Reads and writes data.json. The JSON schema is versioned with schemaVersion.
class JsonStorage
{
public:
    static StorageLoadResult load();

    // Returns the stored UI language without loading the item list. This lets
    // main.cpp install the right translator before user-facing messages are
    // generated.
    static QString storedLanguage();

    // Returns false and fills errorMessage when the file cannot be written.
    static bool save(const QVector<Item> &items,
                     const QString &language,
                     QString *errorMessage);
};
