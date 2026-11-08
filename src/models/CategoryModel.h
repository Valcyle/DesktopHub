#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>
#include <QtQml/qqmlregistration.h>

#include "core/Category.h"

// Stores the user-defined categories.
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use the model exposed by App instead.")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        CategoryIdRole = Qt::UserRole + 1,
        CategoryNameRole,
    };
    Q_ENUM(Roles)

    explicit CategoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    // Returns an empty string on success, or a translated error message.
    Q_INVOKABLE QString addCategory(const QString &name);
    Q_INVOKABLE QString renameCategory(const QString &id, const QString &name);
    Q_INVOKABLE void removeCategory(const QString &id);

    Q_INVOKABLE QVariantMap category(const QString &id) const;
    Q_INVOKABLE bool hasName(const QString &name, const QString &excludeId = QString()) const;

    QVector<Category> categories() const;
    void setCategories(const QVector<Category> &categories);

    Category categoryById(const QString &id) const;
    QHash<QString, QString> nameMap() const;

signals:
    void countChanged();
    void categoriesChanged();

private:
    QString validateName(const QString &name, const QString &excludeId) const;
    int rowForId(const QString &id) const;

    QVector<Category> m_categories;
};
