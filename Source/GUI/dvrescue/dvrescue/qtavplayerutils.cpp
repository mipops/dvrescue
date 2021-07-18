#include "mediaplayer.h"
#include "qtavplayerutils.h"
#include <QtAVPlayer/qavplayer.h>

QtAVPlayerUtils::QtAVPlayerUtils(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<MediaPlayer::Status>();
    qRegisterMetaType<MediaPlayer::PlaybackState>();
}

qint64 QtAVPlayerUtils::displayPosition(QObject *qmlPlayer)
{
    auto player = qmlPlayer->findChild<QAVPlayer*>();
    return player ? player->position() : 0;
}

qreal QtAVPlayerUtils::fps(QObject *qmlPlayer)
{
    // auto player = qmlPlayer->findChild<QAVPlayer*>();
    // return player->statistics().video.frame_rate;
    return 25;
}
