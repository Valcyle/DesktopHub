#pragma once

#include <QObject>
#include <QString>

class QLocalServer;
class QLockFile;

// Enforces a single running instance. The primary instance owns a lock file and
// a local server; later instances ask the primary to raise its window.
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstance(QObject *parent = nullptr);
    ~SingleInstance() override;

    // Returns true when this process became the primary instance.
    bool acquire(QString *errorMessage = nullptr);

    // Used by a secondary instance to ask the primary to show its window.
    bool notifyExisting();

signals:
    void showRequested();

private:
    void handleConnection();

    QString m_serverName;
    QLockFile *m_lockFile = nullptr;
    QLocalServer *m_server = nullptr;
};
