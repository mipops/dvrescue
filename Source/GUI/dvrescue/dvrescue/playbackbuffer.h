#ifndef PLAYBACKBUFFER_H
#define PLAYBACKBUFFER_H

#include <QIODevice>
#include <QMutex>
#include <QObject>

class Buffer: public QIODevice
{
public:
    qint64 readData(char *data, qint64 maxSize) override
    {
        QMutexLocker locker(&m_mutex);
        if (!maxSize)
            return 0;

        QByteArray ba = m_buffer.mid(m_pos, maxSize);
        memcpy(data, ba.data(), ba.size());
        m_pos += ba.size();
        return ba.size();
    }

    qint64 writeData(const char *data, qint64 maxSize) override
    {
        QMutexLocker locker(&m_mutex);
        QByteArray ba(data, maxSize);
        m_buffer.append(ba);
        return ba.size();
    }

    bool atEnd() const override
    {
        return false;

        QMutexLocker locker(&m_mutex);
        return m_pos >= m_size;
    }

    qint64 pos() const override
    {
        QMutexLocker locker(&m_mutex);
        return m_pos;
    }

    bool seek(qint64 pos) override
    {
        QMutexLocker locker(&m_mutex);
        m_pos = pos;
        return true;
    }

    qint64 size() const override
    {
        return m_size;
    }

    qint64 m_size = 0;
    qint64 m_pos = 0;
    QByteArray m_buffer;
    mutable QMutex m_mutex;
};

class BufferSequential : public Buffer
{
    Q_OBJECT
public:
    BufferSequential() {
        open(QIODevice::ReadWrite);
    }
    bool isSequential() const override
    {
        return true;
    }

    bool atEnd() const override
    {
        return false;
    }

public Q_SLOTS:
    void write(const QByteArray &data);
};

#endif // PLAYBACKBUFFER_H
