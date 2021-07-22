#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QTimer>
#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavaudiooutput.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
class QAbstractVideoSurface;
class Source : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractVideoSurface *videoSurface READ videoSurface WRITE setVideoSurface)
public:
    explicit Source(QObject *parent = 0) : QObject(parent) { }
    virtual ~Source() { }

    QAbstractVideoSurface* videoSurface() const { return m_surface; }
    void setVideoSurface(QAbstractVideoSurface *surface)
    {
        m_surface = surface;
    }

    QAbstractVideoSurface *m_surface = nullptr;
};
#endif

class QDeclarativeVideoOutput;
class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeVideoOutput* videoOutput READ videoOutput WRITE setVideoOutput NOTIFY videoOutputChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal videoFrameRate READ videoFrameRate NOTIFY videoFrameRateChanged)
public:
    enum PlaybackState {
        StoppedState,
        PlayingState,
        PausedState
    };
    Q_ENUM(PlaybackState)

    enum Status
    {
        NoMedia,
        LoadingMedia,
        SeekingMedia,
        LoadedMedia,
        EndOfMedia,
        InvalidMedia
    };
    Q_ENUM(Status)

    explicit MediaPlayer(QObject *parent = nullptr);

    QDeclarativeVideoOutput *videoOutput() const;
    void setVideoOutput(QDeclarativeVideoOutput *newVideoOutput);

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void seek(quint64 pos);

    Status status() const;
    PlaybackState playbackState() const;
    qint64 duration() const;
    qint64 position() const;
    qreal videoFrameRate() const;

    QUrl source() const;
    void setSource(const QUrl& newSource);

Q_SIGNALS:
    void videoOutputChanged();
    void positionChanged();
    void statusChanged(MediaPlayer::Status status);
    void playbackStateChanged(MediaPlayer::PlaybackState state);
    void durationChanged(qint64 duration);
    void sourceChanged(const QUrl &url);
    void seekFinished();
    void videoFrameRateChanged(qreal frameRate);

private:
    QAVPlayer* player;
    QAVAudioOutput audioOutput;
    QDeclarativeVideoOutput *m_videoOutput;

    QTimer t;
    qint64 prevPos { 0 };
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    Source mediaSource;
#endif //
};

#endif // MEDIAPLAYER_H
