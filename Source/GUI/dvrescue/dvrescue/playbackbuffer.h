#ifndef PLAYBACKBUFFER_H
#define PLAYBACKBUFFER_H

#include <QIODevice>
#include <QMutex>
#include <QObject>
#include <QDebug>
#include <QWaitCondition>

class Buffer: public QIODevice
{
public:
    qint64 readData(char *data, qint64 maxSize) override
    {
        // qDebug() << "readData: " << maxSize;
        QMutexLocker locker(&m_mutex);
        if (!maxSize)
            return 0;

        // qDebug() << "m_buffer.size = " << m_buffer.size();
        auto sizeToCopy = qMin(maxSize, (qint64) m_buffer.size());
        memcpy(data, m_buffer.data() + m_pos, sizeToCopy);

        m_buffer.remove(m_pos, sizeToCopy);

        // qDebug() << "m_buffer.size = " << m_buffer.size();

        if(m_buffer.size() < 1024 * 1024) {
            // qDebug() << "buffer size is: " << m_buffer.size() << ", awaking writer";
            m_bufferNotFull.wakeAll();
        }

        // qDebug() << "readData: data read, buffer: " << ba.size();
        m_bytesRead += sizeToCopy;
        // qDebug() << "readData: data read, total read: " << m_bytesRead;

        return sizeToCopy;
    }

    qint64 writeData(const char *data, qint64 maxSize) override
    {
        // qDebug() << "writeData: " << maxSize;
        QMutexLocker locker(&m_mutex);
        if(m_buffer.size() > 1024 * 1024) {
            // qDebug() << "writeData: wait on buffer not full";
            m_bufferNotFull.wait(&m_mutex);
            // qDebug() << "writeData: buffer not full";
        }

        // qDebug() << "before append: " << m_buffer.size() << ba.size();
        QByteArray ba(data, maxSize);
        m_buffer.append(ba);
        // qDebug() << "m_buffer: " << m_buffer.size();

        m_bytesWritten += ba.size();
        // qDebug() << "writeData: total data written: " << m_bytesWritten;
        return ba.size();
    }

    bool atEnd() const override
    {
        return false;
    }

    qint64 pos() const override
    {
        return 0;
    }

    bool seek(qint64 pos) override
    {
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
    QWaitCondition m_bufferNotFull;
    quint64 m_bytesRead { 0 };
    quint64 m_bytesWritten { 0 };
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
