#include "mediaplayer.h"
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <private/qdeclarativevideooutput_p.h>

class PlanarVideoBuffer : public QAbstractPlanarVideoBuffer
{
public:
    PlanarVideoBuffer(const QAVVideoFrame &frame, HandleType type = NoHandle)
        : QAbstractPlanarVideoBuffer(type), m_frame(frame)
    {
    }

    MapMode mapMode() const override { return m_mode; }
    int map(MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4]) override
    {
        if (m_mode != NotMapped || mode == NotMapped)
            return 0;

        auto mapData = m_frame.map();
        m_mode = mode;
        if (numBytes)
            *numBytes = mapData.size;

        int i = 0;
        for (; i < 4; ++i) {
            if (!mapData.bytesPerLine[i])
                break;

            bytesPerLine[i] = mapData.bytesPerLine[i];
            data[i] = mapData.data[i];
        }

        return i;
    }
    void unmap() override { m_mode = NotMapped; }

private:
    QAVVideoFrame m_frame;
    MapMode m_mode = NotMapped;
};

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent), player(new QAVPlayer(this))
{    
    connect(player, &QAVPlayer::stateChanged, [this](auto state) {
        qDebug() << "MediaPlayer => state changed" << (PlaybackState) state;
        Q_EMIT playbackStateChanged((PlaybackState) state);
    });
    connect(player, &QAVPlayer::mediaStatusChanged, [this](auto mediaStatus) {
        qDebug() << "MediaPlayer => status changed" << (Status) mediaStatus;
        Q_EMIT statusChanged((Status) mediaStatus);
    });
    connect(player, &QAVPlayer::seeked, [this](auto pos) {
        Q_UNUSED(pos);
        qDebug() << "MediaPlayer => seek finished at" << pos;
        Q_EMIT seekFinished();
    });
    connect(player, &QAVPlayer::durationChanged, [this](auto duration) {
        qDebug() << "MediaPlayer => duration changed" << duration;
        Q_EMIT durationChanged(duration);
    });
    connect(player, &QAVPlayer::sourceChanged, [this](auto source) {
        qDebug() << "MediaPlayer => source changed" << source;
        Q_EMIT sourceChanged(source);
    });
    connect(player, &QAVPlayer::videoFrameRateChanged, [this](auto frameRate) {
        qDebug() << "MediaPlayer => video frame rate changed" << frameRate;
        Q_EMIT videoFrameRateChanged(frameRate);
    });

    t.setInterval(100);
    connect(&t, &QTimer::timeout, [this]() {
        if(player->position() != prevPos) {
            prevPos = player->position();
            Q_EMIT positionChanged();
        }
    });
    t.start();
}

QDeclarativeVideoOutput *MediaPlayer::videoOutput() const
{
    return m_videoOutput;
}

void MediaPlayer::setVideoOutput(QDeclarativeVideoOutput *newVideoOutput)
{
    if (m_videoOutput == newVideoOutput)
        return;
    m_videoOutput = newVideoOutput;

    auto vo = m_videoOutput;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    vo->setSource(&mediaSource);
    auto videoSurface = mediaSource.m_surface;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto videoSurface = vo->videoSurface();
#endif

    QObject::connect(player, &QAVPlayer::videoFrame, player, [videoSurface](const QAVVideoFrame &frame) {
        qDebug() << "got frame";

        QVideoFrame::PixelFormat pf = QVideoFrame::Format_Invalid;
        switch (frame.frame()->format)
        {
        case AV_PIX_FMT_YUV420P:
            pf = QVideoFrame::Format_YUV420P;
            break;
        case AV_PIX_FMT_NV12:
            pf = QVideoFrame::Format_NV12;
            break;
        case AV_PIX_FMT_D3D11:
            pf = QVideoFrame::Format_NV12;
            break;
        default:
            if (frame)
                qDebug() << "format not supported: " << frame.frame()->format;
        }

        QVideoFrame videoFrame(new PlanarVideoBuffer(frame), frame.size(), pf);
        if (!videoSurface->isActive())
            videoSurface->start({videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType()});
        if (videoSurface->isActive())
            videoSurface->present(videoFrame);
    });

    QObject::connect(player, &QAVPlayer::audioFrame, player, [this](const QAVAudioFrame &frame) {
        audioOutput.play(frame);
    });

    Q_EMIT videoOutputChanged();
}

void MediaPlayer::play()
{
    player->play();
}

void MediaPlayer::pause()
{
    player->pause();
}

void MediaPlayer::stop()
{
    player->stop();
}

void MediaPlayer::seek(quint64 pos)
{
    qDebug() << "seek to " << pos;
    player->seek(pos);
}

MediaPlayer::Status MediaPlayer::status() const
{
    return (Status) player->mediaStatus();
}

MediaPlayer::PlaybackState MediaPlayer::playbackState() const
{
    return (PlaybackState) player->state();
}

qint64 MediaPlayer::duration() const
{
    return player->duration();
}

qint64 MediaPlayer::position() const
{
    return player->position();
}

qreal MediaPlayer::videoFrameRate() const
{
    return player->videoFrameRate();
}

QUrl MediaPlayer::source() const
{
    return player->source();
}

void MediaPlayer::setSource(const QUrl &newSource)
{
    player->setSource(newSource);
}
