#include "models/ItemFilterModel.h"

#include "models/ItemModel.h"

ItemFilterModel::ItemFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);

    connect(this, &QAbstractItemModel::rowsInserted, this, &ItemFilterModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &ItemFilterModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &ItemFilterModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &ItemFilterModel::countChanged);
}

void ItemFilterModel::setSourceItemModel(ItemModel *model)
{
    setSourceModel(model);
}

QString ItemFilterModel::searchText() const
{
    return m_searchText;
}

void ItemFilterModel::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
    emit searchTextChanged();
}

QString ItemFilterModel::categoryId() const
{
    return m_categoryId;
}

void ItemFilterModel::setCategoryId(const QString &id)
{
    if (m_categoryId == id) {
        return;
    }

    m_categoryId = id;
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
    emit categoryIdChanged();
}

int ItemFilterModel::count() const
{
    return rowCount();
}

QString ItemFilterModel::itemIdAt(int row) const
{
    const QModelIndex filtered = index(row, 0);
    if (!filtered.isValid()) {
        return {};
    }
    return filtered.data(ItemModel::ItemIdRole).toString();
}

bool ItemFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!sourceIndex.isValid()) {
        return false;
    }

    const QString categoryId =
        sourceIndex.data(ItemModel::CategoryIdRole).toString();

    if (!m_categoryId.isEmpty()) {
        if (m_categoryId == QLatin1String("__uncategorized__")) {
            if (!categoryId.isEmpty()) {
                return false;
            }
        } else if (categoryId != m_categoryId) {
            return false;
        }
    }

    const QString query = m_searchText.trimmed();
    if (query.isEmpty()) {
        return true;
    }

    const QString name = sourceIndex.data(ItemModel::NameRole).toString();
    const QString categoryName = sourceIndex.data(ItemModel::CategoryNameRole).toString();
    return name.contains(query, Qt::CaseInsensitive)
        || categoryName.contains(query, Qt::CaseInsensitive);
}
