#include "services/JsonStorage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QStringList>

#include <algorithm>

#include "core/AppPaths.h"

namespace {
QString tr(const char *text)
{
    return QCoreApplication::translate("JsonStorage", text);
}

QJsonObject itemToJson(const Item &item)
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), item.id);
    object.insert(QStringLiteral("name"), item.name);
    object.insert(QStringLiteral("type"), item.type);
    object.insert(QStringLiteral("target"), item.target);
    object.insert(QStringLiteral("categoryId"),
                  item.categoryId.isEmpty() ? QJsonValue() : QJsonValue(item.categoryId));
    object.insert(QStringLiteral("favorite"), item.favorite);
    object.insert(QStringLiteral("createdAt"),
                  item.createdAt.toUTC().toString(Qt::ISODate));
    return object;
}

QJsonObject categoryToJson(const Category &category)
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), category.id);
    object.insert(QStringLiteral("name"), category.name);
    return object;
}
} // namespace

QString JsonStorage::storedLanguage()
{
    QFile file(AppPaths::dataFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return {};
    }

    const QJsonObject settings =
        document.object().value(QStringLiteral("settings")).toObject();
    const QString language = settings.value(QStringLiteral("language")).toString();
    if (language == QLatin1String("ja") || language == QLatin1String("en")) {
        return language;
    }

    return {};
}

StorageLoadResult JsonStorage::load()
{
    StorageLoadResult result;

    const QString path = AppPaths::dataFilePath();
    QFile file(path);
    if (!file.exists()) {
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        result.warning = tr("The data file could not be opened: %1").arg(file.errorString());
        result.readOnly = true;
        return result;
    }

    const QByteArray payload = file.readAll();
    file.close();

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        // Never delete user data: move the broken file aside and start empty.
        const QString corruptPath = AppPaths::corruptFilePath();
        if (QFile::rename(path, corruptPath)) {
            result.corruptBackupPath = corruptPath;
            result.warning = tr("The data file was corrupt. It was moved to:\n%1\n\n"
                                "An empty dashboard has been started.")
                                 .arg(corruptPath);
        } else {
            result.warning = tr("The data file was corrupt and could not be moved aside. "
                                "Changes will not be saved in this session.");
            result.readOnly = true;
        }
        return result;
    }

    const QJsonObject root = document.object();
    const QJsonValue versionValue = root.value(QStringLiteral("schemaVersion"));
    if (!versionValue.isDouble()) {
        result.warning = tr("The data file does not contain a valid schema version. "
                            "Changes will not be saved in this session.");
        result.readOnly = true;
        return result;
    }

    const int schemaVersion = versionValue.toInt();
    if (schemaVersion > 1) {
        result.fatalMessage = tr("This data file was created by a newer version of Desktop Hub. "
                                 "Update the application before opening it.");
        result.readOnly = true;
        return result;
    }
    if (schemaVersion < 1) {
        result.fatalMessage = tr("This data file uses an unsupported schema version.");
        result.readOnly = true;
        return result;
    }

    const QJsonObject settings = root.value(QStringLiteral("settings")).toObject();
    const QString language = settings.value(QStringLiteral("language")).toString();
    if (language == QLatin1String("ja") || language == QLatin1String("en")) {
        result.language = language;
    }

    QVector<Category> loadedCategories;
    QSet<QString> categoryIds;
    QSet<QString> categoryNamesLower;
    const QJsonArray categories = root.value(QStringLiteral("categories")).toArray();
    loadedCategories.reserve(categories.size());

    for (const QJsonValue &value : categories) {
        if (!value.isObject()) {
            ++result.skippedCategories;
            continue;
        }

        const QJsonObject object = value.toObject();
        Category category;
        category.id = object.value(QStringLiteral("id")).toString();
        category.name = object.value(QStringLiteral("name")).toString().trimmed();

        const QString nameKey = category.name.toLower();
        if (category.id.isEmpty() || category.name.isEmpty()
            || category.name.size() > 50
            || categoryIds.contains(category.id)
            || categoryNamesLower.contains(nameKey)) {
            ++result.skippedCategories;
            continue;
        }

        categoryIds.insert(category.id);
        categoryNamesLower.insert(nameKey);
        loadedCategories.append(category);
    }

    result.categories = loadedCategories;

    QVector<Item> loadedItems;
    const QJsonArray items = root.value(QStringLiteral("items")).toArray();
    loadedItems.reserve(items.size());

    for (const QJsonValue &value : items) {
        if (!value.isObject()) {
            ++result.skippedItems;
            continue;
        }

        const QJsonObject object = value.toObject();
        Item item;
        item.id = object.value(QStringLiteral("id")).toString();
        item.name = object.value(QStringLiteral("name")).toString().trimmed();
        item.type = object.value(QStringLiteral("type")).toString();
        item.target = object.value(QStringLiteral("target")).toString();
        item.favorite = object.value(QStringLiteral("favorite")).toBool();
        item.categoryId = object.value(QStringLiteral("categoryId")).toString();

        const QString createdAt = object.value(QStringLiteral("createdAt")).toString();
        item.createdAt = QDateTime::fromString(createdAt, Qt::ISODate).toUTC();

        const bool knownType = item.type == QLatin1String("exe")
            || item.type == QLatin1String("lnk")
            || item.type == QLatin1String("folder")
            || item.type == QLatin1String("url");

        const bool duplicateId = std::any_of(
            loadedItems.cbegin(), loadedItems.cend(),
            [&item](const Item &existing) { return existing.id == item.id; });

        if (item.id.isEmpty() || item.name.isEmpty() || item.target.isEmpty()
            || !knownType || !item.createdAt.isValid() || duplicateId) {
            ++result.skippedItems;
            continue;
        }

        if (!item.categoryId.isEmpty() && !categoryIds.contains(item.categoryId)) {
            item.categoryId.clear();
            ++result.uncategorizedItems;
        }

        loadedItems.append(item);
    }

    // Requirements sort items by creation time. Keep the order stable for
    // items that share the same timestamp.
    std::stable_sort(loadedItems.begin(), loadedItems.end(),
                     [](const Item &left, const Item &right) {
                         return left.createdAt < right.createdAt;
                     });

    result.items = loadedItems;

    QStringList warnings;
    if (result.skippedItems > 0) {
        warnings.append(tr("%1 item(s) could not be loaded and were skipped.")
                            .arg(result.skippedItems));
    }
    if (result.skippedCategories > 0) {
        warnings.append(tr("%1 category(ies) could not be loaded and were skipped.")
                            .arg(result.skippedCategories));
    }
    if (result.uncategorizedItems > 0) {
        warnings.append(tr("%1 item(s) referenced a missing category and were "
                           "treated as uncategorized.")
                            .arg(result.uncategorizedItems));
    }
    result.warning = warnings.join(QStringLiteral("\n\n"));

    return result;
}

bool JsonStorage::save(const QVector<Item> &items,
                       const QVector<Category> &categories,
                       const QString &language,
                       QString *errorMessage)
{
    const QString path = AppPaths::dataFilePath();
    const QString directory = QFileInfo(path).absolutePath();
    if (!QDir().mkpath(directory)) {
        if (errorMessage) {
            *errorMessage = tr("The data directory could not be created:\n%1").arg(directory);
        }
        return false;
    }

    // Keep exactly one generation of the previous file. A failure to copy the
    // backup must not prevent the actual save; QSaveFile still protects the
    // new file from partial writes.
    if (QFile::exists(path)) {
        const QString backupPath = AppPaths::backupFilePath();
        QFile::remove(backupPath);
        QFile::copy(path, backupPath);
    }

    QJsonArray itemArray;
    for (const Item &item : items) {
        itemArray.append(itemToJson(item));
    }

    QJsonArray categoryArray;
    for (const Category &category : categories) {
        categoryArray.append(categoryToJson(category));
    }

    QJsonObject settings;
    settings.insert(QStringLiteral("language"), language);
    settings.insert(QStringLiteral("theme"), QStringLiteral("dark"));

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 1);
    root.insert(QStringLiteral("items"), itemArray);
    root.insert(QStringLiteral("categories"), categoryArray);
    root.insert(QStringLiteral("settings"), settings);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = tr("The data file could not be opened for writing:\n%1")
                                .arg(file.errorString());
        }
        return false;
    }

    const QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size()) {
        if (errorMessage) {
            *errorMessage = tr("The data file could not be written:\n%1").arg(file.errorString());
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = tr("The data file could not be saved:\n%1").arg(file.errorString());
        }
        return false;
    }

    return true;
}
