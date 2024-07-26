#include "xmlparsingtest.h"
#include "xmlparser.h"

#include <QtTest>
#include <QFile>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QXmlQuery>
#include <QXmlResultItems>
#endif //
#include <QBuffer>

XmlParsingTest::XmlParsingTest(QObject *parent) : QObject(parent)
{

}

int XmlParsingTest::parse(QString fileName)
{
    QFile file(fileName);
    bool opened = file.open(QIODevice::ReadOnly);

    qDebug() << "file: " << opened << file.size();

    XmlParser parser;
    connect(&parser, &XmlParser::bytesProcessed, [&](auto value) {
        // qDebug() << "bytes processed: " << value;
    });

    int totalFrames = 0;
    connect(&parser, &XmlParser::gotFrame, [&](auto value, auto offset, auto duration) {
        qDebug() << "frame processed: " << value << offset << duration;
        ++totalFrames;
    });
    connect(&parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
    });
    connect(&parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;
    });

    parser.exec(&file);

    return totalFrames;
}

void XmlParsingTest::f1()
{
    auto fileName = ":/testdata/sample.xml";
    auto totalFrames = parse(fileName);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    QXmlQuery query;
    auto expectedTotalFrames = 0;

    {
        QFile file(":/testdata/sample.xml");
        file.open(QIODevice::ReadOnly);
        query.setFocus(&file);
        query.setQuery("declare namespace c = \"https://mediaarea.net/dvrescue\";count(//c:frame)");
        if(query.isValid())
        {
            QXmlResultItems queryResult;
            query.evaluateTo(&queryResult);

            QXmlItem item = queryResult.next();
            if(!item.isNull())
            {
                expectedTotalFrames = item.toAtomicValue().toInt();
                qDebug() << "expectedTotalFrames: " << expectedTotalFrames;
            }
        }
    }

    qDebug() << "totalFrames: " << totalFrames;
    QCOMPARE(totalFrames, expectedTotalFrames);
#endif //
}

void XmlParsingTest::f2()
{

}

void XmlParsingTest::parseFile()
{
    auto totalFrames = parse(fileName());
    qDebug() << "totalFrames: " << totalFrames;
}

QString XmlParsingTest::fileName() const
{
    return m_fileName;
}

void XmlParsingTest::setFileName(const QString &newFileName)
{
    if (m_fileName == newFileName)
        return;
    m_fileName = newFileName;
    emit fileNameChanged();
}
