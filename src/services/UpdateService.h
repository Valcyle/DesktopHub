#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

#include <functional>

class QNetworkAccessManager;

// Checks GitHub Releases for a newer portable build, downloads it, verifies the
// published SHA-256 checksum, and hands the replacement to DesktopHubUpdater.
class UpdateService : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use App.updates instead.")

    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY stateChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY stateChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY stateChanged)
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)

public:
    explicit UpdateService(QObject *parent = nullptr);

    QString currentVersion() const;
    QString latestVersion() const;
    QString releaseNotes() const;
    QString statusText() const;
    QString errorText() const;
    bool available() const;
    bool busy() const;
    int progress() const;

    Q_INVOKABLE void checkForUpdates(bool automatic = false);
    Q_INVOKABLE void downloadAndInstall();

    // Rebuilds translated status text after a language change.
    void retranslateUi();

signals:
    void stateChanged();
    void progressChanged();
    void updateFound(bool automatic);
    void errorOccurred(const QString &message);

private:
    using FetchCallback =
        std::function<void(const QByteArray &, const QString &, int)>;

    void fetch(const QUrl &url, bool trackProgress, FetchCallback callback);
    void handleReleaseFeed(const QByteArray &data,
                           const QString &error,
                           int httpStatus,
                           bool automatic);
    void installZip(const QByteArray &zip);
    void fail(const QString &message);
    void setStatus(const QString &status);

    QNetworkAccessManager *m_network = nullptr;
    QString m_currentVersion;
    QString m_latestVersion;
    QString m_releaseNotes;
    QString m_statusText;
    QString m_errorText;
    QUrl m_zipUrl;
    QUrl m_checksumUrl;
    QString m_zipFileName;
    bool m_available = false;
    bool m_busy = false;
    bool m_userInitiated = false;
    int m_progress = 0;
};
