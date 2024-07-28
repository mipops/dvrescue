#ifndef XMLPARSINGTEST_H
#define XMLPARSINGTEST_H

#include <QObject>

class XmlParsingTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)
public:
    explicit XmlParsingTest(QObject *parent = nullptr);

    QString fileName() const;
    void setFileName(const QString &newFileName);

Q_SIGNALS:
    void fileNameChanged();

private Q_SLOTS:
    void f1();
    void f2();
    void parseFile();

private:
    int parse(QString fileName);

private:
    QString m_fileName;
};

#endif // XMLPARSINGTEST_H
