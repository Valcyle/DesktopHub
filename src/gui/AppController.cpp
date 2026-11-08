#include "gui/AppController.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "core/AppPaths.h"
#include "core/AppSettings.h"
#include "services/JsonStorage.h"
#include "services/Logger.h"

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

bool AppController::readOnly() const
{
    return m_readOnly;
}

bool AppController::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
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
            Logger::warning(QStringLiteral("Could not open URL: %1").arg(item.target));
            emit errorOccurred(tr("Could not open link"),
                               tr("The link could not be opened in your browser:\n%1")
                                   .arg(item.target),
                               false);
        }
        return;
    }

    const QFileInfo info(item.target);
    if (!info.exists()) {
        Logger::warning(QStringLiteral("Item target not found: %1 -> %2")
                            .arg(item.name, item.target));
        emit errorOccurred(tr("Item not found"),
                           tr("The file or folder could not be found:\n%1\n\n"
                              "Edit the item and register it again.")
                               .arg(item.target),
                           false);
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(item.target))) {
        Logger::warning(QStringLiteral("Could not open item: %1").arg(item.target));
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
        setUnsavedChanges(true);
        emit errorOccurred(tr("Changes are not saved"),
                           tr("This session is read-only because the data file "
                              "could not be prepared."),
                           false);
        return false;
    }

    QString error;
    const QString language = m_settings ? m_settings->language() : QStringLiteral("en");
    if (!JsonStorage::save(m_items->items(), m_categories->categories(), language, &error)) {
        setUnsavedChanges(true);
        Logger::error(QStringLiteral("Saving data failed: %1").arg(error));
        emit errorOccurred(tr("Could not save data"), error, true);
        return false;
    }

    setUnsavedChanges(false);
    return true;
}

void AppController::setUnsavedChanges(bool unsaved)
{
    if (m_hasUnsavedChanges == unsaved) {
        return;
    }

    m_hasUnsavedChanges = unsaved;
    emit hasUnsavedChangesChanged();
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

    // Report registered local items whose files or folders no longer exist.
    // Items are kept and can be edited instead of being deleted automatically.
    QStringList missingNames;
    for (const Item &item : result.items) {
        if (!isLocalItemType(item.type)) {
            continue;
        }
        if (!QFileInfo::exists(item.target)) {
            missingNames.append(item.name);
            Logger::warning(QStringLiteral("Missing item target: %1 -> %2")
                                .arg(item.name, item.target));
        }
    }
    if (!missingNames.isEmpty()) {
        QString message = tr("%n item(s) could not be found. Edit them from Apps "
                             "and register them again.",
                             "", missingNames.size());
        const QStringList preview = missingNames.mid(0, 10);
        message += QLatin1Char('\n') + preview.join(QLatin1Char('\n'));
        if (missingNames.size() > preview.size()) {
            message += QLatin1Char('\n')
                + tr("... and %1 more").arg(missingNames.size() - preview.size());
        }
        messages.append(message);
    }

    m_startupWarning = messages.join(QStringLiteral("\n\n"));

    Logger::info(QStringLiteral("Loaded items=%1 categories=%2 from %3")
                     .arg(result.items.size())
                     .arg(result.categories.size())
                     .arg(AppPaths::dataFilePath()));
    if (!m_startupWarning.isEmpty()) {
        Logger::warning(QStringLiteral("Startup notice: %1").arg(m_startupWarning));
    }
    if (m_readOnly) {
        Logger::warning(QStringLiteral("Running in read-only mode."));
    }

    setUnsavedChanges(false);

    m_loading = false;

    emit readOnlyChanged();
    m_home->rebuild();
    emit categoryOptionsChanged();
    emit startupWarningChanged();
}
