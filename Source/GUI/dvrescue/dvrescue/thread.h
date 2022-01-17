#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class Thread : public QThread
{
    Q_OBJECT
    Q_PROPERTY(QObject* worker READ worker WRITE setWorker NOTIFY workerChanged)
    QObject *m_worker;

public:
    explicit Thread(QObject *parent = nullptr);
    ~Thread();

    QObject *worker() const;
    void setWorker(QObject *newWorker);

Q_SIGNALS:

    void workerChanged();
};

#endif // THREAD_H
