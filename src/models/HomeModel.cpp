#include "models/HomeModel.h"

#include <QSet>

#include "core/Item.h"
#include "models/CategoryModel.h"
#include "models/ItemModel.h"

HomeModel::HomeModel(ItemModel *items, CategoryModel *categories, QObject *parent)
    : QAbstractListModel(parent)
    , m_items(items)
    , m_categories(categories)
{
    connect(m_items, &ItemModel::itemsChanged, this, &HomeModel::rebuild);
}

int HomeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant HomeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }

    const Row &row = m_rows.at(index.row());
    switch (role) {
    case IsHeaderRole:
        return row.header;
    case TitleRole:
        return row.title;
    case ItemIdRole:
        return row.itemId;
    case NameRole:
        return row.name;
    case TargetRole:
        return row.target;
    case CategoryNameRole:
        return row.categoryName;
    case FavoriteRole:
        return row.favorite;
    case RevisionRole:
        return row.revision;
    default:
        return {};
    }
}

QHash<int, QByteArray> HomeModel::roleNames() const
{
    return {
        { IsHeaderRole, "isHeader" },
        { TitleRole, "title" },
        { ItemIdRole, "itemId" },
        { NameRole, "name" },
        { TargetRole, "target" },
        { CategoryNameRole, "categoryName" },
        { FavoriteRole, "favorite" },
        { RevisionRole, "revision" },
    };
}

int HomeModel::count() const
{
    return m_itemCount;
}

QString HomeModel::searchText() const
{
    return m_searchText;
}

void HomeModel::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    rebuild();
    emit searchTextChanged();
}

QString HomeModel::itemIdAt(int row) const
{
    if (row < 0 || row >= m_rows.size() || m_rows.at(row).header) {
        return {};
    }
    return m_rows.at(row).itemId;
}

void HomeModel::rebuild()
{
    beginResetModel();

    m_rows.clear();
    m_itemCount = 0;

    if (!m_items || !m_categories) {
        endResetModel();
        emit countChanged();
        return;
    }

    const QVector<Item> items = m_items->items();
    const QVector<Category> categories = m_categories->categories();
    const QHash<QString, QString> categoryNames = m_categories->nameMap();

    QSet<QString> knownCategoryIds;
    for (const Category &category : categories) {
        knownCategoryIds.insert(category.id);
    }

    // Favorites section (favorites also remain visible in their category).
    QVector<Item> favorites;
    for (const Item &item : items) {
        if (item.favorite && matches(item, categoryNames)) {
            favorites.append(item);
        }
    }
    if (!favorites.isEmpty()) {
        appendHeader(tr("Favorites"));
        for (const Item &item : favorites) {
            appendItem(item, categoryNames);
        }
    }

    // One section per category, in the order defined by the category list.
    for (const Category &category : categories) {
        QVector<Item> sectionItems;
        for (const Item &item : items) {
            if (item.categoryId == category.id && matches(item, categoryNames)) {
                sectionItems.append(item);
            }
        }
        if (sectionItems.isEmpty()) {
            continue;
        }

        appendHeader(category.name);
        for (const Item &item : sectionItems) {
            appendItem(item, categoryNames);
        }
    }

    // Uncategorized items (including references to missing categories).
    QVector<Item> uncategorized;
    for (const Item &item : items) {
        const bool known = !item.categoryId.isEmpty() && knownCategoryIds.contains(item.categoryId);
        if (!known && matches(item, categoryNames)) {
            uncategorized.append(item);
        }
    }
    if (!uncategorized.isEmpty()) {
        appendHeader(tr("Uncategorized"));
        for (const Item &item : uncategorized) {
            appendItem(item, categoryNames);
        }
    }

    endResetModel();
    emit countChanged();
}

bool HomeModel::matches(const Item &item, const QHash<QString, QString> &categoryNames) const
{
    const QString query = m_searchText.trimmed();
    if (query.isEmpty()) {
        return true;
    }

    if (item.name.contains(query, Qt::CaseInsensitive)) {
        return true;
    }

    const QString categoryName = categoryNames.value(item.categoryId);
    return categoryName.contains(query, Qt::CaseInsensitive);
}

void HomeModel::appendHeader(const QString &title)
{
    Row row;
    row.header = true;
    row.title = title;
    m_rows.append(row);
}

void HomeModel::appendItem(const Item &item, const QHash<QString, QString> &categoryNames)
{
    Row row;
    row.itemId = item.id;
    row.name = item.name;
    row.target = item.target;
    row.categoryName = categoryNames.value(item.categoryId);
    row.favorite = item.favorite;
    row.revision = item.revision;
    m_rows.append(row);
    ++m_itemCount;
}
