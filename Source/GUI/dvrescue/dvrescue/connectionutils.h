#ifndef CONNECTIONUTILS_H
#define CONNECTIONUTILS_H

#include <QObject>

class ConnectionUtils : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionUtils(QObject *parent = nullptr);

    Q_INVOKABLE bool connectToSlotDirect(QObject* source, const QString& signalName, QObject* target, const QString& slotName);
    Q_INVOKABLE bool connectToSlotQueued(QObject* source, const QString& signalName, QObject* target, const QString& slotName);
Q_SIGNALS:

};

#endif // CONNECTIONUTILS_H
