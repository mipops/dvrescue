#include "qtavplayerutils.h"
#include <QtAVPlayer/qavplayer.h>

QtAVPlayerUtils::QtAVPlayerUtils(QObject *parent) : QObject(parent)
{
}

qint64 QtAVPlayerUtils::displayPosition(QObject *qmlPlayer)
{
    // auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    // return player->displayPosition();

    return 0;
}

void QtAVPlayerUtils::setPauseOnEnd(QObject *qmlPlayer)
{
    // auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    // player->setMediaEndAction(QtAV::MediaEndAction_Pause);

    return;
}

qreal QtAVPlayerUtils::fps(QObject *qmlPlayer)
{
    // auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    // return player->statistics().video.frame_rate;

    return 0;
}
