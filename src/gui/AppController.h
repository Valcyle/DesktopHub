#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

#include "models/ItemModel.h"

class AppSettings;

// QML-facing facade. It owns the item model, loads and saves data, and reports
// user-facing errors through a single signal.
class AppController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(App)
    QML_SINGLETON

    Q_PROPERTY(ItemModel *items READ items CONSTANT)
    Q_PROPERTY(QString startupWarning READ startupWarning NOTIFY startupWarningChanged)
    Q_PROPERTY(bool fatalError READ fatalError NOTIFY startupWarningChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    // Called from main.cpp after the QML engine has created the singleton.
    void initialize(AppSettings *settings);

    ItemModel *items() const;
    QString startupWarning() const;
    bool fatalError() const;

    Q_INVOKABLE void clearStartupWarning();
    Q_INVOKABLE void launchItem(const QString &id);

    // Returns false when the data could not be written. A dialog is requested
    // through errorOccurred in that case.
    Q_INVOKABLE bool save();

    Q_INVOKABLE QString localFile(const QUrl &url) const;
    Q_INVOKABLE QString nameFromTarget(const QString &type, const QString &target) const;

signals:
    void errorOccurred(const QString &title, const QString &message, bool retryable);
    void startupWarningChanged();

private:
    void load();

    ItemModel *m_items = nullptr;
    AppSettings *m_settings = nullptr;
    QString m_startupWarning;
    bool m_fatalError = false;
    bool m_readOnly = false;
    bool m_loading = false;
};
