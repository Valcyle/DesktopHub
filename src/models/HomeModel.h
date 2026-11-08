#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QtQml/qqmlregistration.h>

class CategoryModel;
class ItemModel;
struct Item;

// Builds the Home dashboard as a flat list of section headers and item rows:
// Favorites first, then each category, then uncategorized items.
class HomeModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use the model exposed by App instead.")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    enum Roles {
        IsHeaderRole = Qt::UserRole + 1,
        TitleRole,
        ItemIdRole,
        NameRole,
        TargetRole,
        CategoryNameRole,
        FavoriteRole,
        RevisionRole,
    };
    Q_ENUM(Roles)

    HomeModel(ItemModel *items, CategoryModel *categories, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    QString searchText() const;
    void setSearchText(const QString &text);

    Q_INVOKABLE QString itemIdAt(int row) const;
    Q_INVOKABLE void rebuild();

signals:
    void countChanged();
    void searchTextChanged();

private:
    struct Row
    {
        bool header = false;
        QString title;
        QString itemId;
        QString name;
        QString target;
        QString categoryName;
        bool favorite = false;
        int revision = 0;
    };

    bool matches(const Item &item, const QHash<QString, QString> &categoryNames) const;
    void appendHeader(const QString &title);
    void appendItem(const Item &item, const QHash<QString, QString> &categoryNames);

    ItemModel *m_items = nullptr;
    CategoryModel *m_categories = nullptr;
    QVector<Row> m_rows;
    QString m_searchText;
    int m_itemCount = 0;
};
