#ifndef QTAVPLAYERUTILS_H
#define QTAVPLAYERUTILS_H

#include <QObject>

class QtAVPlayerUtils : public QObject
{
    Q_OBJECT
public:
    explicit QtAVPlayerUtils(QObject *parent = nullptr);

    Q_INVOKABLE qint64 displayPosition(QObject* qmlPlayer);
    Q_INVOKABLE qreal fps(QObject* qmlPlayer);

Q_SIGNALS:

};

#endif // QTAVPLAYERUTILS_H
