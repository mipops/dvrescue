#include "playertest.h"
#include <QTest>
#include <QThread>
#include <QtAVPlayer/QtAVPlayer>
#include <QVideoWidget>

PlayerTest::PlayerTest(QObject *parent)
{
    qRegisterMetaType<QAVAudioFrame>();
}

void PlayerTest::test()
{
    QAVPlayer p;

    QFileInfo file(path());
    p.setSource(file.absoluteFilePath());

    QAVVideoFrame frame;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&frame](const QAVVideoFrame &f) { frame = f; });

    p.play();
    QTRY_VERIFY(frame);

    auto mapData = frame.map();
    QVERIFY(mapData.size > 0);
    QVERIFY(mapData.bytesPerLine[0] > 0);
    QVERIFY(mapData.bytesPerLine[1] > 0);
    QVERIFY(mapData.data[0] != nullptr);
    QVERIFY(mapData.data[1] != nullptr);
}


class VideoRenderer : public QVideoRendererControl
{
public:
    QAbstractVideoSurface *surface() const override
    {
        return m_surface;
    }

    void setSurface(QAbstractVideoSurface *surface) override
    {
        m_surface = surface;
    }

    QAbstractVideoSurface *m_surface = nullptr;
};

class MediaObject;
class MediaService : public QMediaService
{
public:
    MediaService(VideoRenderer *vr, QObject* parent = nullptr)
        : QMediaService(parent)
        , m_renderer(vr)
    {
    }

    QMediaControl* requestControl(const char *name) override
    {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            return m_renderer;

        return nullptr;
    }

    void releaseControl(QMediaControl *) override
    {
    }

    VideoRenderer *m_renderer = nullptr;
};

class MediaObject : public QMediaObject
{
public:
    explicit MediaObject(VideoRenderer *vr, QObject* parent = nullptr)
        : QMediaObject(parent, new MediaService(vr, parent))
    {
    }
};

class VideoWidget : public QVideoWidget
{
public:
    bool setMediaObject(QMediaObject *object) override
    {
        return QVideoWidget::setMediaObject(object);
    }
};


void PlayerTest::testPlaybackFromQIODevice()
{

    VideoRenderer vr;
    VideoWidget w;
    w.show();

    MediaObject mo(&vr);
    w.setMediaObject(&mo);

    QAVPlayer p;

    QAVAudioOutput audioOutput;
    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [&audioOutput](const QAVAudioFrame &frame) { audioOutput.play(frame); });

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&vr](const QAVVideoFrame &frame) {
        if (vr.m_surface == nullptr)
            return;

        QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
        if (!vr.m_surface->isActive() || vr.m_surface->surfaceFormat().frameSize() != videoFrame.size()) {
            QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType());
            vr.m_surface->start(f);
        }
        if (vr.m_surface->isActive())
            vr.m_surface->present(videoFrame);
    });


    QFileInfo fileInfo(path());
    QFile file(path());
    file.open(QFile::ReadOnly);

    // works
    /*
    {
        QAVVideoFrame frame;
        QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&frame](const QAVVideoFrame &f) {
            qDebug() << "got frame";
        });

        p.setSource(fileInfo.fileName(), &file);
        p.stepForward();

        QTest::qWait(1000);
    }
    */

    /*
    class QBufferEx : public QBuffer {
        virtual bool atEnd() const {
            return false;
        }
        virtual qint64 readData(char *data, qint64 maxlen) {
            auto dataRead = 0;
            do {
                //qDebug() << "read: reading...";
                m.lock();
                dataRead = QBuffer::readData(data, maxlen);
                if(dataRead == 0) {
                    //qDebug() << "read: waiting for write... ";
                    wc.wait(&m);
                    //qDebug() << "read: waiting for write... done";
                    m.unlock();
                } else {
                    m.unlock();
                    qDebug() << "read: reading... done";
                    break;
                }
            } while(true);
            return dataRead;
        }

        qint64 writeData(const char *data, qint64 len) {

            //qDebug() << "write: writing...";
            m.lock();
            auto written = QBuffer::writeData(data, len);
            if(written != 0) {
                //qDebug() << "writting... done";
                wc.wakeOne();
            }
            m.unlock();
            return written;
        }

        QMutex m;
        QWaitCondition wc;
    };

    QBufferEx buffer;
    buffer.open(QIODevice::ReadWrite);
    */

    QLocalServer server;
    if(server.listen("ringbuffer")) {
        qDebug() << "listening ringbuffer";
    } else {
        qDebug() << "end of listening ringbuffer";
    }

    class QBufferEx : public QLocalSocket {
    public:
        virtual qint64 readData(char *data, qint64 maxlen) {
            auto dataRead = 0;
            do {
                qDebug() << "read: reading...";
                m.lock();
                auto bytesAvail = bytesAvailable();
                while(bytesAvailable() < maxlen) {
                    wc.wait(&m);
                }
                m.unlock();
                qDebug() << "read: got some data...";

                dataRead = QLocalSocket::readData(data, maxlen);
                if(dataRead == 0) {
                    qDebug() << "read: waiting for write... ";
                    wc.wait(&m);
                    qDebug() << "read: waiting for write... done";
                    m.unlock();
                } else {
                    qDebug() << "read: reading... done";
                    break;
                }
            } while(true);
            return dataRead;
        }

        QMutex m;
        QWaitCondition wc;
    };

    /*
    QBufferEx buffer;
    buffer.setServerName("ringbuffer");
    if(buffer.open(QIODevice::ReadOnly))
    {
        qDebug() << "server opened";
    } else {
        qDebug() << "server not opened";
    }
    */

    QLocalSocket* serverSocket = nullptr;
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        serverSocket = server.nextPendingConnection();

        qDebug() << "serverSocket connected";
    });

    QBufferEx client;
    connect(&client, &QBufferEx::readyRead, [&]() {
       qDebug() << "got more data";
       client.wc.wakeAll();
    });

    client.connectToServer("ringbuffer");
    if (client.waitForConnected(1000))
    {
        qDebug("Connected!");
    }
    else
    {
        qDebug("Not Connected!");
    }

    QTest::qWait(1000);

    QAVVideoFrame frame;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&frame](const QAVVideoFrame &f) {
        qDebug() << "got frame";
    });

    auto &buffer = client;
    p.setSource(QUrl(fileInfo.fileName()).toString(), &buffer);
    // p.play();

    QThreadPool::globalInstance()->start([&file, &serverSocket]() {
        while(!file.atEnd()) {
            auto bytes = file.read(4096);
            qDebug() << "writting";
            serverSocket->write(bytes);
            QThread::msleep(100);
        }
    });

    QThreadPool::globalInstance()->start([&p]() {
        while(true) {
            // qDebug() << "stepping";
            p.stepForward();
            QThread::msleep(1000);
        }
    });

    QTest::qWait(100000);
}

void PlayerTest::testPlaybackFromQIODevice2()
{
    VideoRenderer vr;
    VideoWidget w;
    w.show();

    MediaObject mo(&vr);
    w.setMediaObject(&mo);

    QAVPlayer p;

    QAVAudioOutput audioOutput;
    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [&audioOutput](const QAVAudioFrame &frame) { audioOutput.play(frame); });

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&vr](const QAVVideoFrame &frame) {
        if (vr.m_surface == nullptr)
            return;

        QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
        if (!vr.m_surface->isActive() || vr.m_surface->surfaceFormat().frameSize() != videoFrame.size()) {
            QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType());
            vr.m_surface->start(f);
        }
        if (vr.m_surface->isActive())
            vr.m_surface->present(videoFrame);
    });


    QFileInfo fileInfo(path());
    QFile file(path());
    file.open(QFile::ReadOnly);

    QLocalServer server;
    if(server.listen("ringbuffer")) {
        qDebug() << "listening ringbuffer";
    } else {
        qDebug() << "end of listening ringbuffer";
    }

    class QBufferEx : public QLocalSocket {
    public:
        virtual qint64 readData(char *data, qint64 maxlen) {
            auto dataRead = 0;
            do {
                qDebug() << "read: reading...";
                m.lock();
                auto bytesAvail = bytesAvailable();
                while(bytesAvailable() < maxlen) {
                    wc.wait(&m);
                }
                m.unlock();
                qDebug() << "read: got some data...";
                dataRead = QLocalSocket::readData(data, maxlen);
                if(dataRead == 0) {
                    qDebug() << "read: waiting for write... ";
                    wc.wait(&m);
                    qDebug() << "read: waiting for write... done";
                    m.unlock();
                } else {
                    qDebug() << "read: reading... done";
                    break;
                }
            } while(true);
            return dataRead;
        }

        QMutex m;
        QWaitCondition wc;
    };

    QLocalSocket* serverSocket = nullptr;
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        serverSocket = server.nextPendingConnection();

        qDebug() << "serverSocket connected";
    });

    QBufferEx client;
    connect(&client, &QBufferEx::readyRead, [&]() {
       qDebug() << "got more data";
       client.wc.wakeAll();
    });

    client.connectToServer("ringbuffer", QIODevice::ReadWrite | QIODevice::Unbuffered);
    if (client.waitForConnected(1000))
    {
        qDebug("Connected!");
    }
    else
    {
        qDebug("Not Connected!");
    }

    QTest::qWait(1000);

    QAVVideoFrame frame;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&frame](const QAVVideoFrame &f) {
        qDebug() << "got frame";
    });

    auto &buffer = client;
    p.setSource(fileInfo.fileName(), &buffer);

    QThreadPool::globalInstance()->start([&file, &serverSocket]() {
        while(!file.atEnd()) {
            auto bytes = file.read(4096);
            qDebug() << "writting";
            serverSocket->write(bytes);
            QThread::msleep(1000);
        }
    });

    QThreadPool::globalInstance()->start([&p]() {
        while(true) {
            qDebug() << "stepping";
            p.stepForward();
            QThread::msleep(2000);
        }
    });

    QTest::qWait(100000);
}

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
public:
    BufferSequential() { }
    bool isSequential() const override
    {
        return true;
    }

    bool atEnd() const override
    {
        return false;
    }
};

void PlayerTest::testPlaybackFromQIODevice3()
{
    BufferSequential buffer;
    buffer.open(QIODevice::ReadWrite);

    QFileInfo fileInfo(path());
    QFile file(path());
    file.open(QFile::ReadOnly);

    VideoRenderer vr;
    VideoWidget w;
    w.show();

    MediaObject mo(&vr);
    w.setMediaObject(&mo);

    QAVPlayer p;

    QAVAudioOutput audioOutput;
    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [&audioOutput](const QAVAudioFrame &frame) { audioOutput.play(frame); });

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&vr](const QAVVideoFrame &frame) {
        if (vr.m_surface == nullptr)
            return;

        QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
        if (!vr.m_surface->isActive() || vr.m_surface->surfaceFormat().frameSize() != videoFrame.size()) {
            QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType());
            vr.m_surface->start(f);
        }
        if (vr.m_surface->isActive())
            vr.m_surface->present(videoFrame);
    });

    QObject::connect(&p, &QAVPlayer::errorOccurred, &p, [&](QAVPlayer::Error err, const QString &) {
        if (err == QAVPlayer::ResourceError) {
            buffer.seek(0);
            p.setSource(QLatin1String());
            p.setSource(fileInfo.fileName(), &buffer);
            p.play();
        }
    });

    p.setSource(fileInfo.fileName(), &buffer);
    p.play();

    while(!file.atEnd()) {
        auto bytes = file.read(4 * 1024);
        buffer.write(bytes);
        QTest::qWait(50);
    }
}

void PlayerTest::testPlaybackFromQIODevice4()
{
    QFileInfo fileInfo(path());
    QFile file(path());
    file.open(QFile::ReadOnly);

    Buffer buffer;
    buffer.m_size = file.size();
    buffer.open(QIODevice::ReadWrite);

    VideoRenderer vr;
    VideoWidget w;
    w.show();

    MediaObject mo(&vr);
    w.setMediaObject(&mo);

    QAVPlayer p;

    QAVAudioOutput audioOutput;
    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [&audioOutput](const QAVAudioFrame &frame) { audioOutput.play(frame); });

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&vr](const QAVVideoFrame &frame) {
        if (vr.m_surface == nullptr)
            return;

        QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
        if (!vr.m_surface->isActive() || vr.m_surface->surfaceFormat().frameSize() != videoFrame.size()) {
            QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType());
            vr.m_surface->start(f);
        }
        if (vr.m_surface->isActive())
            vr.m_surface->present(videoFrame);
    });

    p.setSource(fileInfo.fileName(), &buffer);
    p.play();

    QThreadPool::globalInstance()->start([&file, &buffer]() {
        while(!file.atEnd()) {
            auto bytes = file.read(1 * 1024);
            buffer.write(bytes);
            buffer.readyRead();
            // QTest::qWait(50);
        }
    });

    QTest::qWait(30000);
}

void PlayerTest::testMultipleSources()
{
    qDebug() << "testMultipleSources";

    QAVPlayer p;
    QAVAudioOutput out;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [this](const QAVVideoFrame &f) {
        qDebug() << "got video frame";
    });

    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [this, &out](const QAVAudioFrame &f) {
        qDebug() << "got audio frame";
        out.play(f);
    });

    QFileInfo file(path());
    qDebug() << "setSource: " << file.absoluteFilePath();
    p.setSource(file.absoluteFilePath());
    p.play();
    QTest::qWait(100);

    QFileInfo file2(path2());

    qDebug() << "setSource: " << file2.absoluteFilePath();
    p.setSource(file2.absoluteFilePath());
    p.play();
    QTest::qWait(100);
}

const QString &PlayerTest::path() const
{
    return m_path;
}

void PlayerTest::setPath(const QString &newPath)
{
    if (m_path == newPath)
        return;
    m_path = newPath;
    Q_EMIT pathChanged();
}

const QString &PlayerTest::path2() const
{
    return m_path2;
}

void PlayerTest::setPath2(const QString &newPath2)
{
    if (m_path2 == newPath2)
        return;
    m_path2 = newPath2;
    Q_EMIT pathChanged();
}
