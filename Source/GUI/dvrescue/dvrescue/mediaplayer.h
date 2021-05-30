#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
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

    Q_ENUMS(PlaybackState)
    Q_ENUMS(Status)
public:
    enum PlaybackState {
        StoppedState,
        PlayingState,
        PausedState
    };

    enum Status
    {
        UnknownMediaStatus,
        NoMedia,
        LoadingMedia, // when source is set
        LoadedMedia, // if auto load and source is set. player is stopped state
        StalledMedia, // insufficient buffering or other interruptions (timeout, user interrupt)
        BufferingMedia, // NOT IMPLEMENTED
        BufferedMedia, // when playing //NOT IMPLEMENTED
        EndOfMedia, // Playback has reached the end of the current media. The player is in the StoppedState.
        InvalidMedia // what if loop > 0 or stopPosition() is not mediaStopPosition()?
    };

    explicit MediaPlayer(QObject *parent = nullptr);

    QDeclarativeVideoOutput *videoOutput() const;
    void setVideoOutput(QDeclarativeVideoOutput *newVideoOutput);

    Q_INVOKABLE void play();
    Status status() const;
    void setStatus(Status status);

    PlaybackState playbackState() const;
    void setPlaybackState(PlaybackState playbackState);

    const QUrl &source() const;
    void setSource(const QUrl &newSource);

Q_SIGNALS:
    void videoOutputChanged();
    void positionChanged();

    void statusChanged();

    void playbackStateChanged();

    void sourceChanged();

private:
    QAVPlayer p;
    QAVAudioOutput audioOutput;
    QDeclarativeVideoOutput *m_videoOutput;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    Source mediaSource;
#endif //
    Status m_status;
    PlaybackState m_playbackState;
    QUrl m_source;
};

#endif // MEDIAPLAYER_H
