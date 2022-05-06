#include <QApplication>
#include <QTest>
#include <QPair>
#include <QCommandLineParser>
#include <memory>

#include "dummytest.h"
#include "datamodeltest.h"
#include "xmlparsingtest.h"
#include "playertest.h"
#include "csvparsingtest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    std::map<QString, std::unique_ptr<QObject>> tests;

    tests.emplace("datamodel", std::make_unique<DataModelTest>());
    tests.emplace("xmlparsing", std::make_unique<XmlParsingTest>());
    tests.emplace("csvparsing", std::make_unique<CsvParsingTest>());
    tests.emplace("player", std::make_unique<PlayerTest>());
    tests.emplace("dummy", std::make_unique<DummyTest>());

    QString selectedTest;
    QList<QPair<QString, QString>> props;

    auto args = app.arguments();
    if(args.length() >= 3 && args[1] == "-t")
    {
        selectedTest = args[2];
        args.removeAt(2);
        args.removeAt(1);

        for(auto it = args.begin(); it != args.end();)
        {
            if((*it) == "-prop") {
                auto nextIt = it + 1;
                auto keyValue = (*nextIt).split("=");
                auto key = keyValue.takeFirst();
                auto value = keyValue.join("=");

                props.append(QPair<QString, QString>(key, value));

                it = args.erase(it);
                it = args.erase(it);
            } else {
                ++it;
            }
        }
    }

    if(!selectedTest.isEmpty())
    {
        auto testIt = tests.find(selectedTest);
        if(testIt != tests.end())
        {
            auto testObject = testIt->second.get();
            for(auto prop : props) {
                testObject->setProperty(prop.first.toUtf8().constData(), prop.second);
            }

            return QTest::qExec(testObject, args);
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
