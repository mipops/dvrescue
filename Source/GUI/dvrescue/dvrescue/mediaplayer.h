#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QtQml/QQmlParserStatus>
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

class QAVPlayer;
class QDeclarativeVideoOutput;
class MediaPlayer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QDeclarativeVideoOutput* videoOutput READ videoOutput WRITE setVideoOutput NOTIFY videoOutputChanged)
    Q_PROPERTY(MediaStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal videoFrameRate READ videoFrameRate NOTIFY videoFrameRateChanged)
public:
    enum State {
        StoppedState,
        PlayingState,
        PausedState
    };
    Q_ENUM(State)

    enum MediaStatus
    {
        NoMedia,
        LoadedMedia,
        EndOfMedia,
        InvalidMedia
    };
    Q_ENUM(MediaStatus)

    explicit MediaPlayer(QObject *parent = nullptr);

    QDeclarativeVideoOutput *videoOutput() const;
    void setVideoOutput(QDeclarativeVideoOutput *newVideoOutput);

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(quint64 pos);
    Q_INVOKABLE void stepForward();
    Q_INVOKABLE void clear();

    MediaStatus status() const;
    State state() const;
    qint64 duration() const;
    qint64 position() const;
    qreal videoFrameRate() const;

    QUrl source() const;
    void setSource(const QUrl& newSource);

Q_SIGNALS:
    void videoOutputChanged();
    void positionChanged();
    void statusChanged(MediaPlayer::MediaStatus status);
    void stateChanged(MediaPlayer::State state);
    void durationChanged(qint64 duration);
    void sourceChanged(const QUrl &url);
    void seekFinished();
    void videoFrameRateChanged(qreal frameRate);
    void stopped(qint64 pos);

private:
    QAVPlayer* player;
    QScopedPointer<QAVAudioOutput> audioOutput;
    QDeclarativeVideoOutput *m_videoOutput;

    QTimer t;
    qint64 prevPos { 0 };
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    Source mediaSource;
#endif //

    // QQmlParserStatus interface
public:
    void classBegin();
    void componentComplete();
};

#endif // MEDIAPLAYER_H
