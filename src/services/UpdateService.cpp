#include "services/UpdateService.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include <QVector>
#include <QVersionNumber>

#include "services/Logger.h"

namespace {
constexpr auto productionFeed =
    "https://api.github.com/repos/Valcyle/DesktopHub/releases?per_page=20";

struct ReleaseInfo
{
    QString version;
    QString notes;
    QString zipName;
    QUrl zipUrl;
    QUrl checksumUrl;
    QVersionNumber parsedVersion;

    bool isValid() const
    {
        return !parsedVersion.isNull() && !zipName.isEmpty()
            && !zipUrl.isEmpty() && !checksumUrl.isEmpty();
    }
};

QString normalizedVersion(QString tag)
{
    tag = tag.trimmed();
    if (tag.startsWith(QLatin1Char('v')) || tag.startsWith(QLatin1Char('V'))) {
        tag.remove(0, 1);
    }
    return tag;
}

ReleaseInfo parseRelease(const QJsonObject &release)
{
    ReleaseInfo info;

    QString tag = release.value(QStringLiteral("tag_name")).toString();
    if (tag.isEmpty()) {
        tag = release.value(QStringLiteral("version")).toString();
    }

    info.version = normalizedVersion(tag);
    info.parsedVersion = QVersionNumber::fromString(info.version);
    info.notes = release.value(QStringLiteral("body")).toString();
    if (info.notes.isEmpty()) {
        info.notes = release.value(QStringLiteral("notes")).toString();
    }

    const QJsonArray assets = release.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        const QString apiUrl = asset.value(QStringLiteral("url")).toString();
        const QString browserUrl =
            asset.value(QStringLiteral("browser_download_url")).toString();
        // Prefer the API asset endpoint: it has been more reliable than the
        // public release download URL and supports the octet-stream header.
        const QString url = apiUrl.isEmpty() ? browserUrl : apiUrl;

        if (url.isEmpty()) {
            continue;
        }

        if (info.zipUrl.isEmpty() && name.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive)) {
            info.zipName = name;
            info.zipUrl = QUrl(url);
        } else if (info.checksumUrl.isEmpty()
                   && (name == QLatin1String("SHA256SUMS.txt")
                       || name.endsWith(QLatin1String(".sha256"), Qt::CaseInsensitive))) {
            info.checksumUrl = QUrl(url);
        }
    }

    return info;
}

QUrl resolveFeedUrl()
{
    const QByteArray overrideUrl = qgetenv("DESKTOPHUB_UPDATE_FEED");
    if (!overrideUrl.isEmpty()) {
        const QString value = QString::fromLocal8Bit(overrideUrl);
        if (!value.contains(QLatin1String("://"))) {
            return QUrl::fromLocalFile(value);
        }
        return QUrl(value);
    }

    return QUrl(QString::fromLatin1(productionFeed));
}

QString checksumFor(const QString &checksumText, const QString &fileName)
{
    const QStringList lines = checksumText.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList parts = line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        QString listedName = parts.last();
        if (listedName.startsWith(QLatin1Char('*'))) {
            listedName.remove(0, 1);
        }

        if (listedName == fileName) {
            return parts.first();
        }
    }
    return {};
}

void removeStaleUpdateDirectories()
{
    const QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir tempDir(tempPath);
    const QStringList names = tempDir.entryList(
        { QStringLiteral("DesktopHub-update-*") }, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &name : names) {
        // Best effort: the updater may still be finishing from a previous run.
        QDir(tempDir.filePath(name)).removeRecursively();
    }
}
} // namespace

UpdateService::UpdateService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_currentVersion(QCoreApplication::applicationVersion())
{
    m_statusText = tr("Not checked yet.");
    removeStaleUpdateDirectories();
}

QString UpdateService::currentVersion() const
{
    return m_currentVersion;
}

QString UpdateService::latestVersion() const
{
    return m_latestVersion;
}

QString UpdateService::releaseNotes() const
{
    return m_releaseNotes;
}

QString UpdateService::statusText() const
{
    return m_statusText;
}

QString UpdateService::errorText() const
{
    return m_errorText;
}

bool UpdateService::available() const
{
    return m_available;
}

bool UpdateService::busy() const
{
    return m_busy;
}

int UpdateService::progress() const
{
    return m_progress;
}

void UpdateService::checkForUpdates(bool automatic)
{
    if (m_busy) {
        return;
    }

    m_busy = true;
    m_errorText.clear();
    m_progress = 0;
    m_userInitiated = !automatic;
    setStatus(tr("Checking for updates..."));

    const QUrl feedUrl = resolveFeedUrl();
    Logger::info(QStringLiteral("Checking for updates: %1").arg(feedUrl.toString()));

    fetch(feedUrl, false, [this, automatic](const QByteArray &data,
                                            const QString &error,
                                            int httpStatus) {
        handleReleaseFeed(data, error, httpStatus, automatic);
    });
}

void UpdateService::downloadAndInstall()
{
    if (!m_available || m_busy) {
        return;
    }

    m_busy = true;
    m_errorText.clear();
    m_progress = 0;
    m_userInitiated = true;
    setStatus(tr("Downloading update..."));
    emit stateChanged();

    fetch(m_checksumUrl, false, [this](const QByteArray &checksumData,
                                       const QString &checksumError,
                                       int) {
        if (!checksumError.isEmpty()) {
            fail(tr("Could not download the checksum: %1").arg(checksumError));
            return;
        }

        const QString expected = checksumFor(
            QString::fromUtf8(checksumData), m_zipFileName);
        if (expected.isEmpty()) {
            fail(tr("The published checksum does not include the update archive."));
            return;
        }

        fetch(m_zipUrl, true, [this, expected](const QByteArray &zip,
                                               const QString &zipError,
                                               int) {
            if (!zipError.isEmpty()) {
                fail(tr("Could not download the update: %1").arg(zipError));
                return;
            }

            const QString actual = QString::fromLatin1(
                QCryptographicHash::hash(zip, QCryptographicHash::Sha256).toHex());
            if (actual.compare(expected, Qt::CaseInsensitive) != 0) {
                fail(tr("The downloaded update failed verification."));
                return;
            }

            m_progress = 100;
            emit progressChanged();
            installZip(zip);
        });
    });
}

void UpdateService::retranslateUi()
{
    m_errorText.clear();
    if (m_busy) {
        return;
    }

    m_statusText = m_available
        ? tr("Update available: %1").arg(QStringLiteral("v") + m_latestVersion)
        : tr("Not checked yet.");
    emit stateChanged();
}

void UpdateService::fetch(const QUrl &url, bool trackProgress, FetchCallback callback)
{
    if (url.isLocalFile()) {
        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            callback({}, tr("Could not read %1").arg(file.errorString()), 0);
            return;
        }

        const QByteArray data = file.readAll();
        if (trackProgress) {
            m_progress = 100;
            emit progressChanged();
        }
        callback(data, {}, 200);
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("DesktopHub/%1").arg(m_currentVersion));

    if (url.path().contains(QLatin1String("/releases/assets/"))) {
        // Release asset endpoints return the binary only with this header.
        request.setRawHeader("Accept", "application/octet-stream");
    } else if (url.host().compare(QLatin1String("api.github.com"),
                                  Qt::CaseInsensitive) == 0) {
        request.setRawHeader("Accept", "application/vnd.github+json");
    } else {
        request.setRawHeader("Accept", "*/*");
    }

    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(20000);

    QNetworkReply *reply = m_network->get(request);

    if (trackProgress) {
        connect(reply, &QNetworkReply::downloadProgress, this,
                [this](qint64 received, qint64 total) {
                    m_progress = total > 0
                        ? static_cast<int>((received * 100) / total)
                        : -1;
                    emit progressChanged();
                });
    }

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        const QByteArray data = reply->readAll();
        const int httpStatus = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            callback({}, reply->errorString(), httpStatus);
        } else {
            callback(data, {}, httpStatus);
        }
        reply->deleteLater();
    });
}

void UpdateService::handleReleaseFeed(const QByteArray &data,
                                      const QString &error,
                                      int httpStatus,
                                      bool automatic)
{
    // A repository without any published release returns 404, which is not an
    // error for the user.
    if (httpStatus == 404) {
        m_busy = false;
        m_available = false;
        setStatus(tr("No updates have been published yet."));
        emit stateChanged();
        return;
    }

    if (!error.isEmpty()) {
        fail(tr("Update check failed: %1").arg(error));
        return;
    }

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        fail(tr("The update feed could not be read."));
        return;
    }

    // GitHub returns an array for the releases list. A single object is still
    // accepted so local test feeds stay easy to write.
    QVector<QJsonObject> releases;
    if (document.isArray()) {
        for (const QJsonValue &value : document.array()) {
            if (value.isObject()) {
                releases.append(value.toObject());
            }
        }
    } else if (document.isObject()) {
        releases.append(document.object());
    } else {
        fail(tr("The update feed could not be read."));
        return;
    }

    ReleaseInfo selected;
    for (const QJsonObject &release : releases) {
        const ReleaseInfo info = parseRelease(release);
        if (!info.isValid()) {
            continue;
        }

        if (!selected.isValid() || info.parsedVersion > selected.parsedVersion) {
            selected = info;
        }
    }

    if (!selected.isValid()) {
        m_busy = false;
        m_available = false;
        setStatus(tr("No updates have been published yet."));
        emit stateChanged();
        return;
    }

    const QVersionNumber current = QVersionNumber::fromString(m_currentVersion);
    if (current.isNull()) {
        fail(tr("The update feed contains an invalid version."));
        return;
    }

    m_busy = false;

    if (selected.parsedVersion <= current) {
        m_available = false;
        m_zipUrl.clear();
        m_zipFileName.clear();
        m_checksumUrl.clear();
        setStatus(tr("You are using the latest version (%1).")
                      .arg(QStringLiteral("v") + m_currentVersion));
        Logger::info(QStringLiteral("Update check: already up to date (%1).")
                         .arg(m_currentVersion));
        emit stateChanged();
        return;
    }

    m_latestVersion = selected.version;
    m_releaseNotes = selected.notes;
    m_available = true;
    m_zipFileName = selected.zipName;
    m_zipUrl = selected.zipUrl;
    m_checksumUrl = selected.checksumUrl;
    m_progress = 0;
    setStatus(tr("Update available: %1").arg(QStringLiteral("v") + selected.version));
    Logger::info(QStringLiteral("Update available: %1 -> %2")
                     .arg(m_currentVersion, selected.version));
    emit updateFound(automatic);
    emit stateChanged();
}

void UpdateService::installZip(const QByteArray &zip)
{
    const QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString tempRoot = QDir(tempPath)
        .filePath(QStringLiteral("DesktopHub-update-%1").arg(m_latestVersion));

    QDir(tempRoot).removeRecursively();
    if (!QDir().mkpath(tempRoot)) {
        fail(tr("The update could not be prepared in the temporary folder."));
        return;
    }

    const QString zipPath = QDir(tempRoot).filePath(QStringLiteral("update.zip"));
    QFile zipFile(zipPath);
    if (!zipFile.open(QIODevice::WriteOnly)
        || zipFile.write(zip) != zip.size()
        || !zipFile.flush()) {
        fail(tr("The update archive could not be written."));
        return;
    }
    zipFile.close();

    const QString extractDir = QDir(tempRoot).filePath(QStringLiteral("files"));
    QDir().mkpath(extractDir);

    // Windows 11 ships bsdtar, which can extract ZIP archives.
    QProcess tar;
    tar.setProgram(QStringLiteral("tar.exe"));
    tar.setArguments({ QStringLiteral("-xf"), zipPath, QStringLiteral("-C"), extractDir });
    tar.start();
    if (!tar.waitForStarted(5000) || !tar.waitForFinished(120000)
        || tar.exitCode() != 0) {
        Logger::error(QStringLiteral("Update extraction failed: %1")
                          .arg(QString::fromLocal8Bit(tar.readAllStandardError())));
        fail(tr("The update could not be extracted."));
        return;
    }

    const QString applicationDir = QCoreApplication::applicationDirPath();
    const QString updaterSource =
        QDir(applicationDir).filePath(QStringLiteral("DesktopHubUpdater.exe"));
    if (!QFile::exists(updaterSource)) {
        fail(tr("This build cannot self-update because DesktopHubUpdater.exe is missing."));
        return;
    }

    const QString updaterPath =
        QDir(tempRoot).filePath(QStringLiteral("DesktopHubUpdater.exe"));
    QFile::remove(updaterPath);
    if (!QFile::copy(updaterSource, updaterPath)) {
        fail(tr("The updater could not be started."));
        return;
    }

    const QStringList arguments = {
        QStringLiteral("--pid"), QString::number(QCoreApplication::applicationPid()),
        QStringLiteral("--source"), extractDir,
        QStringLiteral("--target"), applicationDir,
        QStringLiteral("--exe"), QStringLiteral("DesktopHub.exe"),
        QStringLiteral("--log"), QDir(tempRoot).filePath(QStringLiteral("update.log")),
    };

    if (!QProcess::startDetached(updaterPath, arguments)) {
        fail(tr("The updater could not be started."));
        return;
    }

    setStatus(tr("Installing update... Desktop Hub will restart."));
    Logger::info(QStringLiteral("Updater started for version %1.").arg(m_latestVersion));

    // Let the UI paint the final status before the process exits.
    QTimer::singleShot(300, QCoreApplication::instance(), &QCoreApplication::quit);
}

void UpdateService::fail(const QString &message)
{
    m_busy = false;
    m_progress = 0;
    m_errorText = message;
    m_statusText = tr("Update failed.");
    Logger::warning(QStringLiteral("Update error: %1").arg(message));
    emit stateChanged();
    emit progressChanged();
    if (m_userInitiated) {
        emit errorOccurred(message);
    }
}

void UpdateService::setStatus(const QString &status)
{
    m_statusText = status;
    emit stateChanged();
}
