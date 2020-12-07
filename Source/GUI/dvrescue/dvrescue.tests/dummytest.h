#ifndef DUMMYTEST_H
#define DUMMYTEST_H

#include <QObject>

class DummyTest : public QObject
{
    Q_OBJECT
public:
    explicit DummyTest(QObject *parent = nullptr);

Q_SIGNALS:

private Q_SLOTS:
    void f3();
    void testtest();
};

#endif // DUMMYTEST_H
