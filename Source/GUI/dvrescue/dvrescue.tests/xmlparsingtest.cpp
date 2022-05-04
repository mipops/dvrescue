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

void XmlParsingTest::f1()
{
    QFile file(":/sample.xml");
    file.open(QIODevice::ReadOnly);

    qDebug() << "file: " << file.size();

    XmlParser parser;
    connect(&parser, &XmlParser::bytesProcessed, [&](auto value) {
        // qDebug() << "bytes processed: " << value;
    });

    int totalFrames = 0;
    connect(&parser, &XmlParser::gotFrame, [&](auto value) {
        // qDebug() << "frame processed: " << value;
        ++totalFrames;
    });
    connect(&parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
    });
    connect(&parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;
    });

    parser.exec(&file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    QXmlQuery query;
    auto expectedTotalFrames = 0;

    {
        QFile file(":/sample.xml");
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
