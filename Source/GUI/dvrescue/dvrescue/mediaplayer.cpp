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

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent)
{
    connect(&p, &QAVPlayer::stateChanged, [this](auto state) {
        Q_EMIT playbackStateChanged((PlaybackState) state);
    });
    connect(&p, &QAVPlayer::mediaStatusChanged, [this](auto mediaStatus) {
        Q_EMIT statusChanged((Status) mediaStatus);
    });
    connect(&p, &QAVPlayer::seeked, [this](auto pos) {
        Q_UNUSED(pos);
        qDebug() << "seek finished at" << pos;
        Q_EMIT seekFinished();
    });
    connect(&p, &QAVPlayer::durationChanged, [this](auto duration) {
        Q_EMIT durationChanged(duration);
    });
    connect(&p, &QAVPlayer::sourceChanged, [this](auto source) {
        Q_EMIT sourceChanged(source);
    });
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

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [videoSurface](const QAVVideoFrame &frame) {
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
            qDebug() << "frame.frame()->format: " << frame.frame()->format;
        }

        if(pf != QVideoFrame::Format_Invalid) {
            QVideoFrame videoFrame(new PlanarVideoBuffer(frame), frame.size(), pf);
            if (!videoSurface->isActive())
                videoSurface->start({videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType()});
            if (videoSurface->isActive())
                videoSurface->present(videoFrame);
        }
    });

    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [this](const QAVAudioFrame &frame) {
        audioOutput.play(frame);
    });

    Q_EMIT videoOutputChanged();
}

void MediaPlayer::play()
{
    p.play();
}

void MediaPlayer::pause()
{
    p.pause();
}

void MediaPlayer::seek(quint64 pos)
{
    qDebug() << "seek to " << pos;
    p.seek(pos);
}

MediaPlayer::Status MediaPlayer::status() const
{
    return (Status) p.mediaStatus();
}

MediaPlayer::PlaybackState MediaPlayer::playbackState() const
{
    return (PlaybackState)  p.state();
}

qint64 MediaPlayer::duration() const
{
    return p.duration();
}

const QUrl &MediaPlayer::source() const
{
    return p.source();
}

void MediaPlayer::setSource(const QUrl &newSource)
{
    p.setSource(newSource);
}
