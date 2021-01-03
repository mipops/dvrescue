#ifndef GRAPHMODELTEST_H
#define GRAPHMODELTEST_H

#include <QObject>

class GraphModelTest : public QObject
{
    Q_OBJECT
public:
    explicit GraphModelTest(QObject *parent = nullptr);

private Q_SLOTS:
    void test_info();

Q_SIGNALS:

};

#endif // GRAPHMODELTEST_H
