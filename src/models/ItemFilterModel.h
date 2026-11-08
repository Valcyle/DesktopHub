#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#include <QtQml/qqmlregistration.h>

class ItemModel;

// Filters the item list by a text query and an optional category.
// The special category id "__uncategorized__" selects items without a category.
class ItemFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use the model exposed by App instead.")

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString categoryId READ categoryId WRITE setCategoryId NOTIFY categoryIdChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ItemFilterModel(QObject *parent = nullptr);

    void setSourceItemModel(ItemModel *model);

    QString searchText() const;
    void setSearchText(const QString &text);

    QString categoryId() const;
    void setCategoryId(const QString &id);

    int count() const;

    Q_INVOKABLE QString itemIdAt(int row) const;

signals:
    void searchTextChanged();
    void categoryIdChanged();
    void countChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
    QString m_categoryId;
};
