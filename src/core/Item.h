#pragma once

#include <QDateTime>
#include <QString>

// One registered item. This is a plain value type so it can be copied between
// the model and the JSON storage layer.
struct Item
{
    QString id;
    QString name;
    QString type;       // exe, lnk, folder, or url
    QString target;
    QString categoryId; // Empty means uncategorized.
    bool favorite = false;
    QDateTime createdAt;

    // Bumped whenever the item changes. QML uses it to refresh the icon image.
    int revision = 0;
};

inline bool isLocalItemType(const QString &type)
{
    return type == QLatin1String("exe")
        || type == QLatin1String("lnk")
        || type == QLatin1String("folder");
}
