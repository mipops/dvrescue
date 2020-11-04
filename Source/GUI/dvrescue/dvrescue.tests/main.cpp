#include <QObject>
#include <QtQuickTest>
#include <QQmlEngine>
#include <iostream>

class Setup : public QObject
{
    Q_OBJECT
public:

public slots:
    void applicationAvailable()
    {
        qDebug() << "applicationAvailable";
    }
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        qDebug() << "qmlEngineAvailable";
    }
};

QUICK_TEST_MAIN_WITH_SETUP(tests, Setup)

#include "main.moc"
