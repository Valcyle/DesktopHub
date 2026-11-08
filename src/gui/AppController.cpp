#include "gui/AppController.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "core/AppSettings.h"
#include "services/JsonStorage.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_items(new ItemModel(this))
    , m_categories(new CategoryModel(this))
    , m_filteredItems(new ItemFilterModel(this))
    , m_home(new HomeModel(m_items, m_categories, this))
{
    m_filteredItems->setSourceItemModel(m_items);
}

void AppController::initialize(AppSettings *settings)
{
    m_settings = settings;

    // Language changes are part of the settings and must be persisted.
    if (m_settings) {
        connect(m_settings, &AppSettings::languageChanged, this, [this]() { save(); });
    }

    // Every successful item change is written immediately.
    connect(m_items, &ItemModel::itemsChanged, this, [this]() {
        if (!m_loading) {
            save();
        }
    });

    connect(m_categories, &CategoryModel::categoriesChanged,
            this, &AppController::onCategoriesChanged);

    load();
}

ItemModel *AppController::items() const
{
    return m_items;
}

CategoryModel *AppController::categories() const
{
    return m_categories;
}

ItemFilterModel *AppController::filteredItems() const
{
    return m_filteredItems;
}

HomeModel *AppController::home() const
{
    return m_home;
}

QVariantList AppController::categoryOptions() const
{
    QVariantList options;
    const QVector<Category> categories = m_categories->categories();
    options.reserve(categories.size());
    for (const Category &category : categories) {
        options.append(QVariantMap{
            { QStringLiteral("id"), category.id },
            { QStringLiteral("name"), category.name },
        });
    }
    return options;
}

QString AppController::searchText() const
{
    return m_searchText;
}

void AppController::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    m_filteredItems->setSearchText(text);
    m_home->setSearchText(text);
    emit searchTextChanged();
}

QString AppController::startupWarning() const
{
    return m_startupWarning;
}

bool AppController::fatalError() const
{
    return m_fatalError;
}

void AppController::clearStartupWarning()
{
    if (m_startupWarning.isEmpty()) {
        return;
    }
    m_startupWarning.clear();
    emit startupWarningChanged();
}

void AppController::launchItem(const QString &id)
{
    const Item item = m_items->itemById(id);
    if (item.id.isEmpty()) {
        return;
    }

    if (item.type == QLatin1String("url")) {
        if (!QDesktopServices::openUrl(QUrl(item.target))) {
            emit errorOccurred(tr("Could not open link"),
                               tr("The link could not be opened in your browser:\n%1")
                                   .arg(item.target),
                               false);
        }
        return;
    }

    const QFileInfo info(item.target);
    if (!info.exists()) {
        emit errorOccurred(tr("Item not found"),
                           tr("The file or folder could not be found:\n%1\n\n"
                              "Edit the item and register it again.")
                               .arg(item.target),
                           false);
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(item.target))) {
        emit errorOccurred(tr("Could not open item"),
                           tr("The item could not be opened:\n%1").arg(item.target),
                           false);
    }
}

QString AppController::addCategory(const QString &name)
{
    return m_categories->addCategory(name);
}

QString AppController::renameCategory(const QString &id, const QString &name)
{
    return m_categories->renameCategory(id, name);
}

void AppController::deleteCategory(const QString &id)
{
    // Items must become uncategorized before the category disappears.
    m_items->clearCategory(id);
    m_categories->removeCategory(id);
}

bool AppController::save()
{
    if (m_loading) {
        return true;
    }

    if (m_readOnly) {
        emit errorOccurred(tr("Changes are not saved"),
                           tr("This session is read-only because the data file "
                              "could not be prepared."),
                           false);
        return false;
    }

    QString error;
    const QString language = m_settings ? m_settings->language() : QStringLiteral("en");
    if (!JsonStorage::save(m_items->items(), m_categories->categories(), language, &error)) {
        emit errorOccurred(tr("Could not save data"), error, true);
        return false;
    }

    return true;
}

QString AppController::localFile(const QUrl &url) const
{
    return url.isLocalFile() ? url.toLocalFile() : url.toString();
}

QString AppController::nameFromTarget(const QString &type, const QString &target) const
{
    const QString trimmed = target.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    if (type == QLatin1String("url")) {
        QString text = trimmed;
        if (!text.contains(QLatin1String("://"))) {
            text.prepend(QLatin1String("https://"));
        }
        const QUrl url(text, QUrl::StrictMode);
        return url.host().isEmpty() ? trimmed : url.host();
    }

    const QFileInfo info(trimmed);
    if (info.isDir()) {
        const QString name = info.fileName();
        return name.isEmpty() ? QDir(trimmed).dirName() : name;
    }

    return info.completeBaseName().isEmpty() ? info.fileName() : info.completeBaseName();
}

void AppController::refreshTranslations()
{
    m_home->rebuild();
}

void AppController::onCategoriesChanged()
{
    m_items->setCategoryNames(m_categories->nameMap());
    m_items->pruneUnknownCategories();

    emit categoryOptionsChanged();

    if (!m_loading) {
        save();
    }

    m_home->rebuild();
}

void AppController::load()
{
    m_loading = true;

    const StorageLoadResult result = JsonStorage::load();

    // Categories must be known before items are loaded so category references
    // can be resolved to display names.
    m_categories->setCategories(result.categories);
    m_items->setCategoryNames(m_categories->nameMap());
    m_items->setItems(result.items);

    if (m_settings && !result.language.isEmpty()) {
        m_settings->setLanguage(result.language);
    }

    m_readOnly = result.readOnly;
    m_fatalError = !result.fatalMessage.isEmpty();

    QStringList messages;
    if (!result.warning.isEmpty()) {
        messages.append(result.warning);
    }
    if (!result.fatalMessage.isEmpty()) {
        messages.append(result.fatalMessage);
    }
    m_startupWarning = messages.join(QStringLiteral("\n\n"));

    m_loading = false;

    m_home->rebuild();
    emit categoryOptionsChanged();
    emit startupWarningChanged();
}
