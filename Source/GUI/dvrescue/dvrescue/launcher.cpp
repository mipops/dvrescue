#include "launcher.h"
#include <QDebug>

Launcher::Launcher(QObject *parent) : QObject(parent)
{
    // m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&m_process, &QProcess::readyReadStandardOutput, [&] {
        QByteArray output = m_process.readAllStandardOutput();

        qDebug() << "output changed: " << output;
        emit outputChanged(output);
    });

    connect(&m_process, &QProcess::readyReadStandardError, [&] {
        QByteArray output = m_process.readAllStandardError();

        qDebug() << "error changed: " << output;
        emit errorChanged(output);
    });

    connect(&m_process, &QProcess::started, [&] {
        emit processStarted(QString("%0").arg((qlonglong) m_process.pid()));
    });

    connect(&m_process, &QProcess::stateChanged, [&](QProcess::ProcessState state) {
        if(state == QProcess::NotRunning)
            emit processFinished();
    });
}

void Launcher::execute(const QString &cmd)
{
    qDebug() << "launching cmd: " << cmd;

    if(!m_workingDirectory.isEmpty())
    {
        qDebug() << "setting working directory: " << m_workingDirectory;
        m_process.setWorkingDirectory(m_workingDirectory);
    }

    m_process.start(cmd);
}

void Launcher::execute(const QString &app, const QStringList arguments)
{
    qDebug() << "launching process: " << app;

    if(!m_workingDirectory.isEmpty())
    {
        qDebug() << "setting working directory: " << m_workingDirectory;
        m_process.setWorkingDirectory(m_workingDirectory);
    }

    m_process.setProgram(app);

#ifdef Q_OS_WIN
    m_process.setNativeArguments("\"" + arguments.join(" ") + "\"");
#endif //

    m_process.start();
}

void Launcher::write(const QByteArray &data)
{
    if(m_process.state() != QProcess::Running)
    {
        qDebug() << "skip writing to process as process is not running";
        return;
    }

    auto written = m_process.write(data);
    Q_UNUSED(written)

    m_process.waitForBytesWritten();
}

void Launcher::closeWrite()
{
    if(m_process.state() != QProcess::Running)
    {
        qDebug() << "skip closing write to process as process is not running";
        return;
    }

    m_process.closeWriteChannel();
}

bool Launcher::waitForFinished(int msec)
{
    if(m_process.state() != QProcess::Running || m_process.state() != QProcess::Starting)
    {
        qDebug() << "skip waiting for process completion as process is not running";
    }

    return m_process.waitForFinished(msec);
}

QString Launcher::program() const
{
    return m_process.program();
}

QStringList Launcher::arguments() const
{
    return m_process.arguments();
}

void Launcher::kill()
{
    m_process.kill();
}

QString Launcher::workingDirectory() const
{
    return m_workingDirectory;
}

void Launcher::setWorkingDirectory(const QString &value)
{
    if(m_workingDirectory != value)
    {
        m_workingDirectory = value;
        emit workingDirectoryChanged(value);
    }

    qDebug() << "new working directory: " << m_workingDirectory;
}
