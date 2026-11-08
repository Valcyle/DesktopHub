#include "models/CategoryModel.h"

#include <QUuid>

namespace {
constexpr int maximumCategoryNameLength = 50;
}

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_categories.size()) {
        return {};
    }

    const Category &category = m_categories.at(index.row());
    switch (role) {
    case CategoryIdRole:
        return category.id;
    case CategoryNameRole:
        return category.name;
    default:
        return {};
    }
}

QHash<int, QByteArray> CategoryModel::roleNames() const
{
    return {
        { CategoryIdRole, "categoryId" },
        { CategoryNameRole, "categoryName" },
    };
}

int CategoryModel::count() const
{
    return m_categories.size();
}

QString CategoryModel::addCategory(const QString &name)
{
    const QString error = validateName(name, {});
    if (!error.isEmpty()) {
        return error;
    }

    Category category;
    category.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    category.name = name.trimmed();

    const int row = m_categories.size();
    beginInsertRows(QModelIndex(), row, row);
    m_categories.append(category);
    endInsertRows();

    emit countChanged();
    emit categoriesChanged();
    return {};
}

QString CategoryModel::renameCategory(const QString &id, const QString &name)
{
    const int row = rowForId(id);
    if (row < 0) {
        return tr("The category no longer exists.");
    }

    const QString error = validateName(name, id);
    if (!error.isEmpty()) {
        return error;
    }

    m_categories[row].name = name.trimmed();
    const QModelIndex changed = index(row, 0);
    emit dataChanged(changed, changed, { CategoryNameRole });
    emit categoriesChanged();
    return {};
}

void CategoryModel::removeCategory(const QString &id)
{
    const int row = rowForId(id);
    if (row < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_categories.removeAt(row);
    endRemoveRows();

    emit countChanged();
    emit categoriesChanged();
}

QVariantMap CategoryModel::category(const QString &id) const
{
    const int row = rowForId(id);
    if (row < 0) {
        return {};
    }

    return {
        { QStringLiteral("id"), m_categories.at(row).id },
        { QStringLiteral("name"), m_categories.at(row).name },
    };
}

bool CategoryModel::hasName(const QString &name, const QString &excludeId) const
{
    const QString trimmed = name.trimmed();
    for (const Category &category : m_categories) {
        if (!excludeId.isEmpty() && category.id == excludeId) {
            continue;
        }
        if (QString::compare(category.name, trimmed, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

QVector<Category> CategoryModel::categories() const
{
    return m_categories;
}

void CategoryModel::setCategories(const QVector<Category> &categories)
{
    beginResetModel();
    m_categories = categories;
    endResetModel();

    emit countChanged();
    emit categoriesChanged();
}

Category CategoryModel::categoryById(const QString &id) const
{
    const int row = rowForId(id);
    return row < 0 ? Category{} : m_categories.at(row);
}

QHash<QString, QString> CategoryModel::nameMap() const
{
    QHash<QString, QString> names;
    names.reserve(m_categories.size());
    for (const Category &category : m_categories) {
        names.insert(category.id, category.name);
    }
    return names;
}

QString CategoryModel::validateName(const QString &name, const QString &excludeId) const
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || trimmed.size() > maximumCategoryNameLength) {
        return tr("Category name must be 1 to 50 characters.");
    }

    if (hasName(trimmed, excludeId)) {
        return tr("A category with this name already exists.");
    }

    return {};
}

int CategoryModel::rowForId(const QString &id) const
{
    for (int row = 0; row < m_categories.size(); ++row) {
        if (m_categories.at(row).id == id) {
            return row;
        }
    }
    return -1;
}
