#include <QApplication>
#include <QTest>
#include <QPair>
#include <QCommandLineParser>
#include <memory>

#include "dummytest.h"
#include "datamodeltest.h"
#include "xmlparsingtest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    std::map<QString, std::unique_ptr<QObject>> tests;

    tests.emplace("datamodel", std::make_unique<DataModelTest>());
    tests.emplace("xmlparsing", std::make_unique<XmlParsingTest>());
    tests.emplace("dummy", std::make_unique<DummyTest>());

    QString selectedTest;
    auto args = app.arguments();
    if(args.length() >= 3 && args[1] == "-t")
    {
        selectedTest = args[2];
        args.removeAt(2);
        args.removeAt(1);
    }

    if(!selectedTest.isEmpty())
    {
        auto testIt = tests.find(selectedTest);
        if(testIt != tests.end())
        {
            return QTest::qExec(testIt->second.get(), args);
        }
    }
    else
    {
        int result = 0;
        for(auto & test : tests) {
            result |= QTest::qExec(test.second.get(), args);
        }
        return result;
    }

    return 0;
}
