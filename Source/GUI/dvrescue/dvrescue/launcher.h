#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QProcess>

class Launcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged)
    Q_PROPERTY(bool useThread READ useThread WRITE setUseThread NOTIFY useThreadChanged)
public:
    explicit Launcher(QObject *parent = 0);
    ~Launcher();

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &value);

    Q_INVOKABLE bool waitForFinished(int msec);
    Q_INVOKABLE QString program() const;
    Q_INVOKABLE QStringList arguments() const;

    bool useThread() const
    {
        return m_useThread;
    }

Q_SIGNALS:
    void errorOccurred(const QByteArray& error);
    void outputChanged(const QByteArray& output);
    void errorChanged(const QByteArray& output);
    void workingDirectoryChanged(const QString& dir);
    void processStarted(const QString& pid);
    void processFinished();

    void useThreadChanged(bool useThread);

public Q_SLOTS:
    void execute(const QString &cmd);
    void execute(const QString &app, const QStringList arguments);
    void setPaths(const QStringList paths);
    void write(const QByteArray& data);
    void closeWrite();
    void kill();

    void setUseThread(bool useThread)
    {
        if (m_useThread == useThread)
            return;

        m_useThread = useThread;
        Q_EMIT useThreadChanged(m_useThread);
    }

private:
    void applyEnvironment();

private:
    QString m_workingDirectory;
    QStringList m_paths;
    QProcess* m_process = { nullptr };
    QProcess::ProcessState m_processState { QProcess::NotRunning };
    QThread* m_thread = { nullptr };
    bool m_useThread = { false };
};

#endif // LAUNCHER_H
