#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Application-wide settings exposed to QML as the "AppSettings" singleton.
//
// Phase 1 keeps the values in memory only. Persisting them to data.json is
// part of the Phase 2 storage work.
class AppSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString theme READ theme CONSTANT)
    Q_PROPERTY(QString dataLocation READ dataLocation CONSTANT)
    Q_PROPERTY(QString translationFile READ translationFile CONSTANT)
    Q_PROPERTY(bool checkForUpdates READ checkForUpdates WRITE setCheckForUpdates
                   NOTIFY checkForUpdatesChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    QString language() const;
    void setLanguage(const QString &language);

    QString theme() const;
    QString dataLocation() const;
    QString translationFile() const;

    bool checkForUpdates() const;
    void setCheckForUpdates(bool enabled);

signals:
    void languageChanged();
    void checkForUpdatesChanged();

private:
    QString m_language;
    bool m_checkForUpdates = true;
};
