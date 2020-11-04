#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QProcess>

class Launcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged)
public:
    explicit Launcher(QObject *parent = 0);

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &value);

    Q_INVOKABLE bool waitForFinished(int msec);

signals:
    void outputChanged(const QByteArray& output);
    void errorChanged(const QByteArray& output);
    void workingDirectoryChanged(const QString& dir);
    void processStarted(const QString& pid);
    void processFinished();

public slots:
    void execute(const QString &cmd);
    void execute(const QString &app, const QStringList arguments);
    void write(const QByteArray& data);
    void closeWrite();
    void kill();

private:
    QString m_workingDirectory;
    QProcess m_process;
};

#endif // LAUNCHER_H
