#include "thread.h"

Thread::Thread(QObject *parent)
    : QThread(parent)
{
    start();
}

Thread::~Thread()
{
    quit();
    wait();
}

QObject *Thread::worker() const
{
    return m_worker;
}

void Thread::setWorker(QObject *newWorker)
{
    if (m_worker == newWorker)
        return;

    m_worker = newWorker;

    m_worker->setParent(nullptr);
    m_worker->moveToThread(this);
    connect(this, &QThread::finished, m_worker, &QObject::deleteLater);

    Q_EMIT workerChanged();
}
