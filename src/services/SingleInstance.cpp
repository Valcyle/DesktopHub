#include "services/SingleInstance.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>

#include "core/AppPaths.h"
#include "services/Logger.h"

SingleInstance::SingleInstance(QObject *parent)
    : QObject(parent)
{
    // QLocalServer names are global per user session; include a stable hash of
    // the data directory so different installations do not collide.
    const QString dataPath =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QByteArray digest =
        QCryptographicHash::hash(dataPath.toUtf8(), QCryptographicHash::Sha1)
            .toHex()
            .left(12);
    m_serverName = QStringLiteral("DesktopHub-") + QString::fromLatin1(digest);
}

SingleInstance::~SingleInstance()
{
    if (m_server) {
        m_server->close();
    }
    if (m_lockFile) {
        if (m_lockFile->isLocked()) {
            m_lockFile->unlock();
        }
        delete m_lockFile;
    }
}

bool SingleInstance::acquire(QString *errorMessage)
{
    const QString lockPath = AppPaths::lockFilePath();
    QDir().mkpath(QFileInfo(lockPath).absolutePath());

    m_lockFile = new QLockFile(lockPath);
    // A crashed process must not block the application forever.
    m_lockFile->setStaleLockTime(30000);

    if (!m_lockFile->tryLock(0)) {
        if (errorMessage) {
            switch (m_lockFile->error()) {
            case QLockFile::PermissionError:
                *errorMessage = tr("The single-instance lock file could not be created "
                                   "because of a permission error.");
                break;
            case QLockFile::UnknownError:
                *errorMessage = tr("The single-instance lock file could not be created.");
                break;
            case QLockFile::LockFailedError:
            default:
                *errorMessage = tr("Another instance is already running.");
                break;
            }
        }
        return false;
    }

    // The lock is ours, so any leftover server name is stale.
    QLocalServer::removeServer(m_serverName);

    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection,
            this, &SingleInstance::handleConnection);

    if (!m_server->listen(m_serverName)) {
        // The lock still protects the data; the only lost ability is raising
        // the existing window from a second instance.
        Logger::warning(QStringLiteral("Could not start single-instance server: %1")
                            .arg(m_server->errorString()));
    } else {
        Logger::info(QStringLiteral("Primary instance acquired the lock."));
    }

    return true;
}

bool SingleInstance::notifyExisting()
{
    QLocalSocket socket;
    socket.connectToServer(m_serverName);

    if (!socket.waitForConnected(500)) {
        Logger::warning(QStringLiteral("Could not reach the existing instance: %1")
                            .arg(socket.errorString()));
        return false;
    }

    socket.write("show");
    const bool sent = socket.waitForBytesWritten(500);
    socket.disconnectFromServer();

    Logger::info(QStringLiteral("Notified the existing instance."));
    return sent;
}

void SingleInstance::handleConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);

        if (socket->waitForReadyRead(300)) {
            const QByteArray message = socket->readAll();
            if (message.contains("show")) {
                emit showRequested();
            }
        }

        socket->disconnectFromServer();
    }
}
