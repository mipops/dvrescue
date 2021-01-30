#ifndef SETUP_H
#define SETUP_H

#include <QObject>
#include <QDebug>
#include <QQmlEngine>

class Setup : public QObject
{
    Q_OBJECT
public:

public Q_SLOTS:
    void applicationAvailable()
    {
        qDebug() << "applicationAvailable";
    }
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        qDebug() << "qmlEngineAvailable";
    }
};

#endif // SETUP_H
