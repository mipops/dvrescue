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
    void applicationAvailable();
    void qmlEngineAvailable(QQmlEngine *engine);
};

#endif // SETUP_H
