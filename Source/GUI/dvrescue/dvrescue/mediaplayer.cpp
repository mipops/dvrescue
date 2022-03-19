#include "mediaplayer.h"
#include <QtAVPlayer/qavplayer.h>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QDebug>

extern "C" {
#include <libavutil/pixdesc.h>
}

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
    audioOutput.reset(new QAVAudioOutput);

    qRegisterMetaType<MediaPlayer::MediaStatus>();
    qRegisterMetaType<MediaPlayer::State>();

    qRegisterMetaType<QAVPlayer::MediaStatus>();
    qRegisterMetaType<QAVPlayer::State>();

    qDebug() << "MediaPlayer::LoadedMedia: " << MediaPlayer::LoadedMedia << (int) MediaPlayer::LoadedMedia;

    connect(player, &QAVPlayer::stateChanged, player, [this](auto state) {
        qDebug() << "MediaPlayer => state changed" << (State) state;
        Q_EMIT stateChanged((State) state);
    });
    connect(player, &QAVPlayer::mediaStatusChanged, player, [this](auto mediaStatus) {
        qDebug() << "MediaPlayer => status changed" << (MediaStatus) mediaStatus;
        Q_EMIT statusChanged((MediaStatus) mediaStatus);
    });
    connect(player, &QAVPlayer::seeked, player, [this](auto pos) {
        Q_UNUSED(pos);
        qDebug() << "MediaPlayer => seek finished at" << pos;
        Q_EMIT seekFinished();
    });
    connect(player, &QAVPlayer::durationChanged, player, [this](auto duration) {
        qDebug() << "MediaPlayer => duration changed" << duration;
        Q_EMIT durationChanged(duration);
    });
    connect(player, &QAVPlayer::sourceChanged, player, [this](auto source) {
        qDebug() << "MediaPlayer => source changed" << source;
        Q_EMIT sourceChanged(source);
    });
    connect(player, &QAVPlayer::videoFrameRateChanged, player, [this](auto frameRate) {
        qDebug() << "MediaPlayer => video frame rate changed" << frameRate;
        Q_EMIT videoFrameRateChanged(frameRate);
    });
    connect(player, &QAVPlayer::stopped, player, [this](auto pos) {
        qDebug() << "MediaPlayer => stopped at " << pos;
        Q_EMIT stopped(pos);
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

MediaPlayer::~MediaPlayer()
{
    disconnect(player, 0, 0, 0);
}

QQuickItem *MediaPlayer::videoOutput() const
{
    return m_videoOutput;
}

void MediaPlayer::setVideoOutput(QQuickItem *newVideoOutput)
{
    if (m_videoOutput == newVideoOutput)
        return;
    m_videoOutput = newVideoOutput;

    auto vo = m_videoOutput;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    vo->setProperty("source", QVariant::fromValue(&mediaSource));
    auto videoSurface = mediaSource.m_surface;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto videoSurface = vo->property("videoSurface").value<QAbstractVideoSurface*>();
#endif

    auto& p = *player;

    QObject::connect(vo, SIGNAL(sourceRectChanged()), vo, SLOT(update()));

    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [videoSurface](QAVVideoFrame frame) {
        qDebug() << "got video frame";

        QVideoFrame videoFrame = frame;
        if (!videoSurface->isActive())
            videoSurface->start({videoFrame.size(), videoFrame.pixelFormat(), videoFrame.handleType()});
        if (videoSurface->isActive())
            videoSurface->present(videoFrame);
    });

    QObject::connect(player, &QAVPlayer::audioFrame, player, [this](const QAVAudioFrame &frame) {
        if(enableAudio())
            audioOutput->play(frame);
    });

    Q_EMIT videoOutputChanged();
}

void MediaPlayer::play()
{
    qDebug() << "play";
    player->play();
}

void MediaPlayer::pause()
{
    qDebug() << "pause";
    player->pause();
}

void MediaPlayer::stop()
{
    qDebug() << "stop";
    player->stop();
}

void MediaPlayer::seek(quint64 pos)
{
    qDebug() << "seek to " << pos;

    if(!m_ranges.isNull())
        pos += m_ranges.x();

    player->seek(pos);
}

void MediaPlayer::stepForward()
{
    qDebug() << "step forward";
    player->stepForward();
}

void MediaPlayer::stepBackward()
{
    qDebug() << "step backward";
    player->stepBackward();
}

void MediaPlayer::clear()
{
    qDebug() << "clear";
    if(!player->videoStreams().empty())
        Q_EMIT player->videoFrame(QAVVideoFrame());
}

MediaPlayer::MediaStatus MediaPlayer::status() const
{
    return (MediaStatus) player->mediaStatus();
}

MediaPlayer::State MediaPlayer::state() const
{
    return (State) player->state();
}

qint64 MediaPlayer::duration() const
{
    return m_ranges.isNull() ? player->duration() : m_ranges.length();
}

qint64 MediaPlayer::position() const
{
    return m_ranges.isNull() ? player->position() : (player->position() - m_ranges.x());
}

qreal MediaPlayer::videoFrameRate() const
{
    return player->videoFrameRate();
}

QString MediaPlayer::source() const
{
    return player->source();
}

void MediaPlayer::setSource(const QString &newSource)
{
    qDebug() << "new source: " << newSource;

    player->setSource(newSource);
}

void MediaPlayer::classBegin()
{

}

void MediaPlayer::componentComplete()
{

}

const QVector2D &MediaPlayer::ranges() const
{
    return m_ranges;
}

void MediaPlayer::setRanges(const QVector2D &newRanges)
{
    if (m_ranges == newRanges)
        return;
    m_ranges = newRanges;
    Q_EMIT rangesChanged();
}

QIODevice* MediaPlayer::buffer() const
{
    return m_buffer;
}

void MediaPlayer::setBuffer(QIODevice* newBuffer)
{
    if (m_buffer == newBuffer)
        return;
    m_buffer = newBuffer;
    player->setSource("dummy", m_buffer);
    Q_EMIT bufferChanged();
}

bool MediaPlayer::enableAudio() const
{
    return m_enableAudio;
}

void MediaPlayer::setEnableAudio(bool newEnableAudio)
{
    if (m_enableAudio == newEnableAudio)
        return;
    m_enableAudio = newEnableAudio;
    Q_EMIT enableAudioChanged();
}
