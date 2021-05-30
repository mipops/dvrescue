#include "launcher.h"
#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>

Launcher::Launcher(QObject *parent) : QObject(parent)
{
    qDebug() << "launcher created at thread: " << QThread::currentThread();
    // m_process.setProcessChannelMode(QProcess::MergedChannels);

    m_process = new QProcess();
    connect(m_process, &QProcess::errorOccurred, this, [&]{
        auto error = m_process->errorString().toUtf8();

        qDebug() << "error occurred at thread " << QThread::currentThread() << ": " << error;
        Q_EMIT errorOccurred(error);
    });

    connect(m_process, &QProcess::readyReadStandardOutput, [&] {
        QByteArray output = m_process->readAllStandardOutput();

        qDebug() << "output changed at thead " << QThread::currentThread() << ": " << output;
        Q_EMIT outputChanged(output);
    });

    connect(m_process, &QProcess::readyReadStandardError, [&] {
        QByteArray output = m_process->readAllStandardError();

        qDebug() << "error changed " << QThread::currentThread() << ": " << output;
        Q_EMIT errorChanged(output);
    });

    connect(m_process, &QProcess::started, [&] {
        Q_EMIT processStarted(QString("%0").arg((qlonglong) m_process->processId()));
    });

    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "emitting processFinished: " << exitCode << exitStatus << "thread: " << QThread::currentThread();
        Q_EMIT processFinished();
    });

    /*
    connect(m_process, &QProcess::stateChanged, [&](QProcess::ProcessState state) {
        qDebug() << "state changed: " << state;
        if(state == QProcess::NotRunning)
            Q_EMIT processFinished();
    });
    */
}

Launcher::~Launcher()
{
    qDebug() << "entering ~Launcher";

    if(m_thread) {
        if(m_thread->isRunning()) {
            qDebug() << "terminating process...";
            m_process->kill();

            qDebug() << "quiting... ";
            m_thread->quit();
            qDebug() << "quiting...done";

            qDebug() << "waiting...";
            m_thread->wait();
            qDebug() << "waiting...done";
        }

        m_thread->deleteLater();
        m_thread = nullptr;
    } else {
        m_process->deleteLater();
    }

    qDebug() << "exiting ~Launcher";
}

void Launcher::execute(const QString &cmd)
{
    qDebug() << "launching cmd: " << cmd;
    applyEnvironment();

    if(m_useThread) {
        qDebug() << "in a separate thread...";

        m_thread = new QThread;
        m_process->moveToThread(m_thread);

        connect(m_thread, &QThread::finished, this, [this]() {
            m_process->deleteLater();
        }, Qt::DirectConnection);

        connect(m_process, &QProcess::stateChanged, this, [this](QProcess::ProcessState state) {
            Q_UNUSED(state);

            qDebug() << "process: " << &m_process << "state changed: " << m_processState << "=>" << state;
            if(m_processState == QProcess::Starting && state == QProcess::NotRunning) {
                // process wasn't started - incorrectly typed command?

                QByteArray output = "incorrectly typed command";

                qDebug() << "error changed " << QThread::currentThread() << ": " << output;
                Q_EMIT errorChanged(output);

                qDebug() << "QProcess::finished: " << m_process->exitCode() << m_process->exitStatus() << "thread: " << QThread::currentThread();
                Q_EMIT m_process->finished(m_process->exitCode(), m_process->exitStatus());
            }
            m_processState = state;
        }, Qt::DirectConnection);

        connect(m_thread, &QThread::started, this, [this, cmd]() {
            qDebug() << "starting process from thread: " << QThread::currentThread();
            auto appAndArguments = cmd.split(" ", QString::SkipEmptyParts);
            m_process->setProgram(appAndArguments[0]);
            if(appAndArguments.size() > 1) {
                appAndArguments.removeFirst();
                m_process->setArguments(appAndArguments);
            }
            m_process->start();
        }, Qt::DirectConnection);

        qDebug() << "starting thread...";
        m_thread->start();
    } else {

        qDebug() << "starting command" << cmd;

        auto appAndArguments = cmd.split(" ", QString::SkipEmptyParts);
        if(appAndArguments.length() == 0)
            return;

        m_process->setProgram(appAndArguments[0]);
        if(appAndArguments.size() > 1) {
            appAndArguments.removeFirst();
            m_process->setArguments(appAndArguments);
        }
        m_process->start();
    }
}

void Launcher::execute(const QString &app, const QStringList arguments)
{
    qDebug() << "launching process: " << app;
    applyEnvironment();

    m_process->setProgram(app);
    m_process->setArguments(arguments);

    m_process->start();
}

void Launcher::setPaths(const QStringList paths)
{
    m_paths = paths;
}

void Launcher::write(const QByteArray &data)
{
    if(m_process->state() != QProcess::Running)
    {
        qDebug() << "skip writing to process as process is not running";
        return;
    }

    auto written = m_process->write(data);
    Q_UNUSED(written)

    m_process->waitForBytesWritten();
}

void Launcher::closeWrite()
{
    if(m_process->state() != QProcess::Running)
    {
        qDebug() << "skip closing write to process as process is not running";
        return;
    }

    m_process->closeWriteChannel();
}

bool Launcher::waitForFinished(int msec)
{
    if(m_process->state() != QProcess::Running || m_process->state() != QProcess::Starting)
    {
        qDebug() << "skip waiting for process completion as process is not running";
    }

    return m_process->waitForFinished(msec);
}

QString Launcher::program() const
{
    return m_process->program();
}

QStringList Launcher::arguments() const
{
    return m_process->arguments();
}

void Launcher::kill()
{
    m_process->kill();
}

void Launcher::applyEnvironment()
{
    if(!m_workingDirectory.isEmpty())
    {
        qDebug() << "setting working directory: " << m_workingDirectory;
        m_process->setWorkingDirectory(m_workingDirectory);
    }

    if(!m_paths.empty())
    {
        qDebug() << "injecting paths: " << m_paths;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        auto paths = env.value("PATH");

#ifdef Q_OS_WIN
        QStringList winPaths;
        for(auto path : m_paths) {
            winPaths.append(QDir::toNativeSeparators(path));
        }
        m_paths = winPaths;
#endif
        auto newPath = m_paths.join(QDir::listSeparator()) + QDir::listSeparator() + paths;

        qDebug() << "new paths: " << newPath;
        env.insert("PATH", newPath);

        m_process->setProcessEnvironment(env);
    }
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
        Q_EMIT workingDirectoryChanged(value);
    }

    qDebug() << "new working directory: " << m_workingDirectory;
}
