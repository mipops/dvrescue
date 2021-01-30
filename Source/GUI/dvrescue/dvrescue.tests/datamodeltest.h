#ifndef DATAMODELTEST_H
#define DATAMODELTEST_H

#include <QObject>

class DataModelTest : public QObject
{
    Q_OBJECT
public:
    explicit DataModelTest(QObject *parent = nullptr);

private Q_SLOTS:
    void test_info();

Q_SIGNALS:

};

#endif // DATAMODELTEST_H
