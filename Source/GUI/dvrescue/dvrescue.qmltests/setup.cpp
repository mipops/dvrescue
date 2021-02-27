#include "setup.h"
#include <sortfiltertablemodel.h>
#include <mediainfo.h>
#include <fileutils.h>
#include <qqmltablemodel_p.h>
#include <QQuickStyle>

void Setup::applicationAvailable()
{
    qDebug() << "applicationAvailable";

    qmlRegisterType<SortFilterTableModel>("SortFilterTableModel", 1, 0, "SortFilterTableModel");
    qmlRegisterType<QQmlTableModel>("TableModel", 1, 0, "TableModel");
    qmlRegisterType<QQmlTableModelColumn>("TableModelColumn", 1, 0, "TableModelColumn");
    qmlRegisterType<MediaInfo>("MediaInfo", 1, 0, "MediaInfo");

    qmlRegisterSingletonType<FileUtils>("FileUtils", 1, 0, "FileUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        FileUtils *utils = new FileUtils();
        return utils;
    });

    QQuickStyle::setStyle("Material");
}

void Setup::qmlEngineAvailable(QQmlEngine *engine)
{
    qDebug() << "qmlEngineAvailable";
}
