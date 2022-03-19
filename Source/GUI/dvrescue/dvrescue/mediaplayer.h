#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QIODevice>
#include <QTimer>
#include <QVector2D>
#include <QtQml/QQmlParserStatus>
#include <QQuickItem>
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

    Q_PROPERTY(QQuickItem* videoOutput READ videoOutput WRITE setVideoOutput NOTIFY videoOutputChanged)
    Q_PROPERTY(MediaStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QIODevice* buffer READ buffer WRITE setBuffer NOTIFY bufferChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal videoFrameRate READ videoFrameRate NOTIFY videoFrameRateChanged)
    Q_PROPERTY(QVector2D ranges READ ranges WRITE setRanges NOTIFY rangesChanged)
    Q_PROPERTY(bool enableAudio READ enableAudio WRITE setEnableAudio NOTIFY enableAudioChanged)
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
    ~MediaPlayer();

    QQuickItem *videoOutput() const;
    void setVideoOutput(QQuickItem *newVideoOutput);

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(quint64 pos);
    Q_INVOKABLE void stepForward();
    Q_INVOKABLE void stepBackward();
    Q_INVOKABLE void clear();

    MediaStatus status() const;
    State state() const;
    qint64 duration() const;
    qint64 position() const;
    qreal videoFrameRate() const;

    QString source() const;
    void setSource(const QString& newSource);

    const QVector2D &ranges() const;
    void setRanges(const QVector2D &newRanges);

    QIODevice *buffer() const;
    void setBuffer(QIODevice *newBuffer);
    bool enableAudio() const;
    void setEnableAudio(bool newEnableAudio);

Q_SIGNALS:
    void videoOutputChanged();
    void positionChanged();
    void statusChanged(MediaPlayer::MediaStatus status);
    void stateChanged(MediaPlayer::State state);
    void durationChanged(qint64 duration);
    void sourceChanged(const QString &url);
    void seekFinished();
    void videoFrameRateChanged(qreal frameRate);
    void stopped(qint64 pos);    
    void rangesChanged();
    void bufferChanged();
    void enableAudioChanged();

private:
    QAVPlayer* player;
    QScopedPointer<QAVAudioOutput> audioOutput;
    QQuickItem *m_videoOutput;

    QTimer t;
    qint64 prevPos { 0 };
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    Source mediaSource;
#endif //

    QVector2D m_ranges;
    QIODevice* m_buffer { nullptr };

    bool m_enableAudio { true };

    // QQmlParserStatus interface
public:
    void classBegin();
    void componentComplete();
};

#endif // MEDIAPLAYER_H
