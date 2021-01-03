#ifndef XMLPARSINGTEST_H
#define XMLPARSINGTEST_H

#include <QObject>

class XmlParsingTest : public QObject
{
    Q_OBJECT
public:
    explicit XmlParsingTest(QObject *parent = nullptr);

Q_SIGNALS:

private Q_SLOTS:
    void f1();
    void f2();
};

#endif // XMLPARSINGTEST_H
