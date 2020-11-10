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

signals:
    void outputChanged(const QByteArray& output);
    void errorChanged(const QByteArray& output);
    void workingDirectoryChanged(const QString& dir);
    void processStarted(const QString& pid);
    void processFinished();

    void useThreadChanged(bool useThread);

public slots:
    void execute(const QString &cmd);
    void execute(const QString &app, const QStringList arguments);
    void write(const QByteArray& data);
    void closeWrite();
    void kill();

    void setUseThread(bool useThread)
    {
        if (m_useThread == useThread)
            return;

        m_useThread = useThread;
        emit useThreadChanged(m_useThread);
    }

private:
    QString m_workingDirectory;
    QProcess m_process;
    QThread* m_thread = { nullptr };
    bool m_useThread = { false };
};

#endif // LAUNCHER_H
