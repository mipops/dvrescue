#ifndef CSVPARSINGTEST_H
#define CSVPARSINGTEST_H

#include <QObject>

class CsvParsingTest : public QObject
{
    Q_OBJECT
public:
    explicit CsvParsingTest(QObject *parent = nullptr);

Q_SIGNALS:

private Q_SLOTS:
    void f1();
    void f2();
};

#endif // CSVPARSINGTEST_H
