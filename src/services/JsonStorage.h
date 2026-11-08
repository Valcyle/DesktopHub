#pragma once

#include <QString>
#include <QVector>

#include "core/Category.h"
#include "core/Item.h"

struct StorageLoadResult
{
    QVector<Item> items;
    QVector<Category> categories;
    QString language;          // Empty when the stored language is unknown.
    int skippedItems = 0;
    int skippedCategories = 0;
    int uncategorizedItems = 0; // Items whose category no longer exists.
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
                     const QVector<Category> &categories,
                     const QString &language,
                     QString *errorMessage);
};
