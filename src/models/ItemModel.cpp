#include "models/ItemModel.h"

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUuid>

namespace {
constexpr int maximumNameLength = 100;

// Returns a cleaned http/https URL, or an empty string when the input is not
// an acceptable URL. A missing scheme is completed with https://.
QString normalizedUrl(const QString &input)
{
    QString text = input.trimmed();
    if (text.isEmpty()) {
        return {};
    }

    if (!text.contains(QLatin1String("://"))) {
        text.prepend(QLatin1String("https://"));
    }

    const QUrl url(text, QUrl::StrictMode);
    if (!url.isValid() || url.host().isEmpty()) {
        return {};
    }

    const QString scheme = url.scheme().toLower();
    if (scheme != QLatin1String("http") && scheme != QLatin1String("https")) {
        return {};
    }

    return url.toString();
}
} // namespace

ItemModel::ItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    const Item &item = m_items.at(index.row());
    switch (role) {
    case ItemIdRole:
        return item.id;
    case NameRole:
        return item.name;
    case TypeRole:
        return item.type;
    case TargetRole:
        return item.target;
    case CategoryIdRole:
        return item.categoryId;
    case CategoryNameRole:
        return m_categoryNames.value(item.categoryId);
    case FavoriteRole:
        return item.favorite;
    case CreatedAtRole:
        return item.createdAt;
    case RevisionRole:
        return item.revision;
    default:
        return {};
    }
}

QHash<int, QByteArray> ItemModel::roleNames() const
{
    return {
        { ItemIdRole, "itemId" },
        { NameRole, "name" },
        { TypeRole, "type" },
        { TargetRole, "target" },
        { CategoryIdRole, "categoryId" },
        { CategoryNameRole, "categoryName" },
        { FavoriteRole, "favorite" },
        { CreatedAtRole, "createdAt" },
        { RevisionRole, "revision" },
    };
}

int ItemModel::count() const
{
    return m_items.size();
}

QString ItemModel::addItem(const QString &name,
                           const QString &type,
                           const QString &target,
                           const QString &categoryId,
                           bool favorite)
{
    QString normalizedTargetValue;
    const QString error = validateAndNormalize(name, type, target, &normalizedTargetValue);
    if (!error.isEmpty()) {
        return error;
    }

    Item item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.name = name.trimmed();
    item.type = type;
    item.target = normalizedTargetValue;
    item.categoryId = normalizeCategoryId(categoryId);
    item.favorite = favorite;
    item.createdAt = QDateTime::currentDateTimeUtc();

    const int row = m_items.size();
    beginInsertRows(QModelIndex(), row, row);
    m_items.append(item);
    endInsertRows();

    emit countChanged();
    emit itemsChanged();
    return {};
}

QString ItemModel::updateItem(const QString &id,
                              const QString &name,
                              const QString &type,
                              const QString &target,
                              const QString &categoryId,
                              bool favorite)
{
    const int row = rowForId(id);
    if (row < 0) {
        return tr("The item no longer exists.");
    }

    QString normalizedTargetValue;
    const QString error = validateAndNormalize(name, type, target, &normalizedTargetValue);
    if (!error.isEmpty()) {
        return error;
    }

    Item &item = m_items[row];
    item.name = name.trimmed();
    item.type = type;
    item.target = normalizedTargetValue;
    item.categoryId = normalizeCategoryId(categoryId);
    item.favorite = favorite;
    ++item.revision;

    const QModelIndex changed = index(row, 0);
    emit dataChanged(changed, changed,
                     { NameRole, TypeRole, TargetRole, CategoryIdRole, CategoryNameRole,
                       FavoriteRole, RevisionRole });
    emit itemsChanged();
    return {};
}

void ItemModel::removeItem(const QString &id)
{
    const int row = rowForId(id);
    if (row < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();

    emit countChanged();
    emit itemsChanged();
}

void ItemModel::setFavorite(const QString &id, bool favorite)
{
    const int row = rowForId(id);
    if (row < 0 || m_items.at(row).favorite == favorite) {
        return;
    }

    m_items[row].favorite = favorite;
    ++m_items[row].revision;
    const QModelIndex changed = index(row, 0);
    emit dataChanged(changed, changed, { FavoriteRole, RevisionRole });
    emit itemsChanged();
}

QVariantMap ItemModel::item(const QString &id) const
{
    const int row = rowForId(id);
    if (row < 0) {
        return {};
    }

    const Item &entry = m_items.at(row);
    return {
        { QStringLiteral("id"), entry.id },
        { QStringLiteral("name"), entry.name },
        { QStringLiteral("type"), entry.type },
        { QStringLiteral("target"), entry.target },
        { QStringLiteral("categoryId"), entry.categoryId },
        { QStringLiteral("favorite"), entry.favorite },
    };
}

bool ItemModel::hasTarget(const QString &type,
                          const QString &target,
                          const QString &excludeId) const
{
    const QString normalized = normalizedTarget(type, target);
    if (normalized.isEmpty()) {
        return false;
    }

    for (const Item &entry : m_items) {
        if (!excludeId.isEmpty() && entry.id == excludeId) {
            continue;
        }
        if (entry.type == type
            && QString::compare(entry.target, normalized, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

QString ItemModel::itemIdAt(int row) const
{
    if (row < 0 || row >= m_items.size()) {
        return {};
    }
    return m_items.at(row).id;
}

QVector<Item> ItemModel::items() const
{
    return m_items;
}

void ItemModel::setItems(const QVector<Item> &items)
{
    beginResetModel();
    m_items = items;
    for (Item &item : m_items) {
        if (!item.categoryId.isEmpty() && !m_categoryNames.contains(item.categoryId)) {
            item.categoryId.clear();
        }
    }
    endResetModel();

    emit countChanged();
    // No itemsChanged here: loading data must not trigger another save.
}

void ItemModel::setCategoryNames(const QHash<QString, QString> &names)
{
    m_categoryNames = names;

    if (!m_items.isEmpty()) {
        emit dataChanged(index(0, 0), index(m_items.size() - 1, 0),
                         { CategoryNameRole });
    }
}

Item ItemModel::itemById(const QString &id) const
{
    const int row = rowForId(id);
    return row < 0 ? Item{} : m_items.at(row);
}

void ItemModel::clearCategory(const QString &categoryId)
{
    if (categoryId.isEmpty()) {
        return;
    }

    bool changed = false;
    for (Item &item : m_items) {
        if (item.categoryId == categoryId) {
            item.categoryId.clear();
            ++item.revision;
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    emit dataChanged(index(0, 0), index(m_items.size() - 1, 0),
                     { CategoryIdRole, CategoryNameRole, RevisionRole });
    emit itemsChanged();
}

void ItemModel::pruneUnknownCategories()
{
    bool changed = false;
    for (Item &item : m_items) {
        if (!item.categoryId.isEmpty() && !m_categoryNames.contains(item.categoryId)) {
            item.categoryId.clear();
            ++item.revision;
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    emit dataChanged(index(0, 0), index(m_items.size() - 1, 0),
                     { CategoryIdRole, CategoryNameRole, RevisionRole });
    emit itemsChanged();
}

QString ItemModel::normalizedTarget(const QString &type, const QString &target) const
{
    if (type == QLatin1String("url")) {
        return normalizedUrl(target);
    }

    const QString trimmed = target.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }
    if (!QDir::isAbsolutePath(trimmed)) {
        return {};
    }

    return QDir::cleanPath(QFileInfo(trimmed).absoluteFilePath());
}

QString ItemModel::validateAndNormalize(const QString &name,
                                        const QString &type,
                                        const QString &target,
                                        QString *normalizedTargetOut) const
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty() || trimmedName.size() > maximumNameLength) {
        return tr("Name must be 1 to 100 characters.");
    }

    if (type != QLatin1String("exe")
        && type != QLatin1String("lnk")
        && type != QLatin1String("folder")
        && type != QLatin1String("url")) {
        return tr("Unsupported item type.");
    }

    const QString normalized = normalizedTarget(type, target);
    if (normalized.isEmpty()) {
        if (type == QLatin1String("url")) {
            return tr("Enter a valid http or https URL.");
        }
        if (!QDir::isAbsolutePath(target.trimmed())) {
            return tr("Select a file or folder with an absolute path.");
        }
        return tr("The selected file or folder does not exist.");
    }

    if (type == QLatin1String("url")) {
        *normalizedTargetOut = normalized;
        return {};
    }

    const QFileInfo info(normalized);
    if (!info.exists()) {
        return tr("The selected file or folder does not exist.");
    }

    if (type == QLatin1String("folder")) {
        if (!info.isDir()) {
            return tr("Select an existing folder.");
        }
    } else {
        if (!info.isFile()) {
            return tr("Select an existing file.");
        }

        const QString suffix = info.suffix().toLower();
        if (type == QLatin1String("exe") && suffix != QLatin1String("exe")) {
            return tr("Select an existing .exe file.");
        }
        if (type == QLatin1String("lnk") && suffix != QLatin1String("lnk")) {
            return tr("Select an existing .lnk shortcut.");
        }
    }

    *normalizedTargetOut = normalized;
    return {};
}

QString ItemModel::normalizeCategoryId(const QString &categoryId) const
{
    return m_categoryNames.contains(categoryId) ? categoryId : QString();
}

int ItemModel::rowForId(const QString &id) const
{
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items.at(row).id == id) {
            return row;
        }
    }
    return -1;
}
