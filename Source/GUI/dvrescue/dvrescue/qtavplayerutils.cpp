#include "qtavplayerutils.h"
#include <QtAV/AVPlayer.h>

QtAVPlayerUtils::QtAVPlayerUtils(QObject *parent) : QObject(parent)
{
    QtAV::setLogLevel(QtAV::LogOff);
}

qint64 QtAVPlayerUtils::displayPosition(QObject *qmlPlayer)
{
    auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    return player->displayPosition();
}

void QtAVPlayerUtils::setPauseOnEnd(QObject *qmlPlayer)
{
    auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    player->setMediaEndAction(QtAV::MediaEndAction_Pause);
}

qreal QtAVPlayerUtils::fps(QObject *qmlPlayer)
{
    auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    return player->statistics().video.frame_rate;
}
