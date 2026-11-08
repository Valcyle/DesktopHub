#include "gui/AppController.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "core/AppSettings.h"
#include "models/ItemModel.h"
#include "services/JsonStorage.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_items(new ItemModel(this))
{
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

    load();
}

ItemModel *AppController::items() const
{
    return m_items;
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
    if (!JsonStorage::save(m_items->items(), language, &error)) {
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

void AppController::load()
{
    m_loading = true;

    const StorageLoadResult result = JsonStorage::load();
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
    emit startupWarningChanged();
}
