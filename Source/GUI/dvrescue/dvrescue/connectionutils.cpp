#include "connectionutils.h"

ConnectionUtils::ConnectionUtils(QObject *parent) : QObject(parent)
{
}

bool ConnectionUtils::connectToSlotDirect(QObject *source, const QString &signalName, QObject *target, const QString &slotName)
{
    QString signal = QString("2") + signalName;
    QString slot = QString("1") + slotName;

    return connect(source, signal.toUtf8().constData(), target, slot.toUtf8().constData(), Qt::DirectConnection);
}

bool ConnectionUtils::connectToSlotQueued(QObject *source, const QString &signalName, QObject *target, const QString &slotName)
{
    QString signal = QString("2") + signalName;
    QString slot = QString("1") + slotName;

    return connect(source, signal.toUtf8().constData(), target, slot.toUtf8().constData(), Qt::QueuedConnection);
}
