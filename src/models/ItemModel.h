#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>
#include <QtQml/qqmlregistration.h>

#include "core/Item.h"

// Holds the registered items and exposes them to QML.
class ItemModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use the model exposed by App instead.")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        ItemIdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        TargetRole,
        CategoryIdRole,
        CategoryNameRole,
        FavoriteRole,
        CreatedAtRole,
        RevisionRole,
    };
    Q_ENUM(Roles)

    explicit ItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    // Returns an empty string on success, or a translated error message.
    Q_INVOKABLE QString addItem(const QString &name,
                                const QString &type,
                                const QString &target,
                                const QString &categoryId,
                                bool favorite);
    Q_INVOKABLE QString updateItem(const QString &id,
                                   const QString &name,
                                   const QString &type,
                                   const QString &target,
                                   const QString &categoryId,
                                   bool favorite);
    Q_INVOKABLE void removeItem(const QString &id);
    Q_INVOKABLE void setFavorite(const QString &id, bool favorite);

    Q_INVOKABLE QVariantMap item(const QString &id) const;
    Q_INVOKABLE bool hasTarget(const QString &type,
                               const QString &target,
                               const QString &excludeId = QString()) const;
    Q_INVOKABLE QString itemIdAt(int row) const;

    QVector<Item> items() const;
    void setItems(const QVector<Item> &items);
    Item itemById(const QString &id) const;

    // Category names are supplied by the controller so the model can expose
    // the display name and support search without owning the category list.
    void setCategoryNames(const QHash<QString, QString> &names);

    // Used when a category is deleted: affected items become uncategorized.
    void clearCategory(const QString &categoryId);
    void pruneUnknownCategories();

signals:
    void countChanged();
    // Emitted after any successful change so the controller can auto-save.
    void itemsChanged();

private:
    QString normalizedTarget(const QString &type, const QString &target) const;
    QString validateAndNormalize(const QString &name,
                                 const QString &type,
                                 const QString &target,
                                 QString *normalizedTargetOut) const;
    QString normalizeCategoryId(const QString &categoryId) const;
    int rowForId(const QString &id) const;

    QVector<Item> m_items;
    QHash<QString, QString> m_categoryNames;
};
