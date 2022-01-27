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

bool ConnectionUtils::connectToSignalQueued(QObject *source, const QString &signalName, QObject *target, const QString &signal2Name)
{
    QString signal = QString("2") + signalName;
    QString signal2 = QString("2") + signal2Name;

    return connect(source, signal.toUtf8().constData(), target, signal2.toUtf8().constData(), Qt::QueuedConnection);
}
