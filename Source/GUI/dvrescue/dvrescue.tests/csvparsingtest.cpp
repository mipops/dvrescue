#include "csvparsingtest.h"
#include <QDebug>
#include <QFile>
#include <QtTest>
#include <csvparser.h>

CsvParsingTest::CsvParsingTest(QObject *parent)
    : QObject{parent}
{

}

void CsvParsingTest::f1()
{
    for(auto i = 0; i < 10; ++i)
    {
        QFile file(":/testdata/sample.csv");
        file.open(QIODevice::ReadOnly);

        qDebug() << "file: " << file.size();

        CsvParser parser;
        auto totalExpectedEntries = 68420;
        auto entriesCount = 0;
        connect(&parser, &CsvParser::columnsChanged, [&](auto value) {
            entriesCount = 1;
        });

        connect(&parser, &CsvParser::entriesReceived, [&](auto value) {
            ++entriesCount;
        });

        while(!file.atEnd()) {
            auto chunk = file.read(rand() % 4096);
            parser.write(chunk);
        }

        qDebug() << "entriesCount: " << entriesCount << "totalExpectedEntries: " << totalExpectedEntries;
        QVERIFY(totalExpectedEntries == entriesCount);
    }
}

void CsvParsingTest::f2()
{

}
