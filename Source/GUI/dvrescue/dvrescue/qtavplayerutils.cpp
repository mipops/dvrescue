#include "qtavplayerutils.h"
#include <QtAV/AVPlayer.h>

QtAVPlayerUtils::QtAVPlayerUtils(QObject *parent) : QObject(parent)
{

}

qint64 QtAVPlayerUtils::displayPosition(QObject *qmlPlayer)
{
    auto player = qmlPlayer->findChild<QtAV::AVPlayer*>();
    return player->displayPosition();
}
