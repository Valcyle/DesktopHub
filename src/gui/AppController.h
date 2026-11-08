#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include "models/CategoryModel.h"
#include "models/HomeModel.h"
#include "models/ItemFilterModel.h"
#include "models/ItemModel.h"
#include "services/UpdateService.h"

class AppSettings;

// QML-facing facade. It owns the item/category models, loads and saves data,
// applies search filters, and reports user-facing errors through one signal.
class AppController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(App)
    QML_SINGLETON

    Q_PROPERTY(ItemModel *items READ items CONSTANT)
    Q_PROPERTY(CategoryModel *categories READ categories CONSTANT)
    Q_PROPERTY(ItemFilterModel *filteredItems READ filteredItems CONSTANT)
    Q_PROPERTY(HomeModel *home READ home CONSTANT)
    Q_PROPERTY(UpdateService *updates READ updates CONSTANT)
    Q_PROPERTY(QVariantList categoryOptions READ categoryOptions NOTIFY categoryOptionsChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString startupWarning READ startupWarning NOTIFY startupWarningChanged)
    Q_PROPERTY(bool fatalError READ fatalError NOTIFY startupWarningChanged)
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY hasUnsavedChangesChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    // Called from main.cpp after the QML engine has created the singleton.
    void initialize(AppSettings *settings);

    ItemModel *items() const;
    CategoryModel *categories() const;
    ItemFilterModel *filteredItems() const;
    HomeModel *home() const;
    UpdateService *updates() const;
    QVariantList categoryOptions() const;

    QString searchText() const;
    void setSearchText(const QString &text);

    QString startupWarning() const;
    bool fatalError() const;
    bool readOnly() const;
    bool hasUnsavedChanges() const;

    Q_INVOKABLE void clearStartupWarning();
    Q_INVOKABLE void launchItem(const QString &id);

    Q_INVOKABLE QString addCategory(const QString &name);
    Q_INVOKABLE QString renameCategory(const QString &id, const QString &name);
    Q_INVOKABLE void deleteCategory(const QString &id);

    // Returns false when the data could not be written. A dialog is requested
    // through errorOccurred in that case.
    Q_INVOKABLE bool save();

    Q_INVOKABLE QString localFile(const QUrl &url) const;
    Q_INVOKABLE QString nameFromTarget(const QString &type, const QString &target) const;

    // Rebuilds translated content after main.cpp installs a new translator.
    void refreshTranslations();

signals:
    void errorOccurred(const QString &title, const QString &message, bool retryable);
    void startupWarningChanged();
    void categoryOptionsChanged();
    void searchTextChanged();
    void readOnlyChanged();
    void hasUnsavedChangesChanged();

private:
    void load();
    void onCategoriesChanged();
    void setUnsavedChanges(bool unsaved);

    ItemModel *m_items = nullptr;
    CategoryModel *m_categories = nullptr;
    ItemFilterModel *m_filteredItems = nullptr;
    HomeModel *m_home = nullptr;
    UpdateService *m_updates = nullptr;
    AppSettings *m_settings = nullptr;
    QString m_searchText;
    QString m_startupWarning;
    bool m_fatalError = false;
    bool m_readOnly = false;
    bool m_loading = false;
    bool m_hasUnsavedChanges = false;
};
