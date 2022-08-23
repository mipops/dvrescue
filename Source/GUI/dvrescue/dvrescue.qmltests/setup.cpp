#include "setup.h"
#include <sortfiltertablemodel.h>
#include <mediainfo.h>
#include <fileutils.h>
#include <clipboard.h>
#include <settingsutils.h>
#include <qqmltablemodel_p.h>
#include <QQuickStyle>
#include <launcher.h>
#include <thread.h>
#include <filewriter.h>
#include <datamodel.h>
#include <mediaplayer.h>
#include <playbackbuffer.h>
#include <qwtquick2plot.h>
#include <csvparser.h>
#include <imageutils.h>
#include <captureerrorplotdatamodel.h>

void Setup::applicationAvailable()
{
    qDebug() << "applicationAvailable";

    qmlRegisterType<Launcher>("Launcher", 0, 1, "Launcher");
    qmlRegisterType<Thread>("Thread", 0, 1, "Thread");
    qmlRegisterType<FileWriter>("FileWriter", 0, 1, "FileWriter");
    qmlRegisterType<CsvParser>("CsvParser", 0, 1, "CsvParser");
    qmlRegisterType<DataModel>("DataModel", 1, 0, "DataModel");
    qmlRegisterType<CaptureErrorPlotDataModel>("CaptureErrorPlotDataModel", 1, 0, "CaptureErrorPlotDataModel");
    qmlRegisterType<MediaInfo>("MediaInfo", 1, 0, "MediaInfo");
    qmlRegisterType<MediaPlayer>("QtAVMediaPlayer", 1, 0, "QtAVMediaPlayer");
    qmlRegisterType<BufferSequential>("MediaPlayerBuffer", 1, 0, "MediaPlayerBuffer");
    qmlRegisterType<QwtQuick2Plot>("QwtQuick2", 1, 0, "QwtQuick2Plot");
    qmlRegisterType<QwtQuick2PlotCurve>("QwtQuick2", 1, 0, "QwtQuick2PlotCurve");
    qmlRegisterType<QwtQuick2PlotGrid>("QwtQuick2", 1, 0, "QwtQuick2PlotGrid");
    qmlRegisterType<QwtQuick2PlotPicker>("QwtQuick2", 1, 0, "QwtQuick2PlotPicker");
    qmlRegisterType<QwtQuick2PlotLegend>("QwtQuick2", 1, 0, "QwtQuick2PlotLegend");
    qmlRegisterType<SortFilterTableModel>("SortFilterTableModel", 1, 0, "SortFilterTableModel");
    qmlRegisterType<QQmlTableModel>("TableModel", 1, 0, "TableModel");
    qmlRegisterType<QQmlTableModelColumn>("TableModelColumn", 1, 0, "TableModelColumn");

    qRegisterMetaType<MarkerInfo>();
    qRegisterMetaType<QList<MarkerInfo>>();
    qRegisterMetaType<QAbstractTableModel*>();
    qRegisterMetaType<QProcess::ProcessState>();
    qRegisterMetaType<QProcess::ExitStatus>();
    qRegisterMetaType<QProcess::ProcessError>();

    qmlRegisterSingletonType<FileUtils>("FileUtils", 1, 0, "FileUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        FileUtils *utils = new FileUtils();
        return utils;
    });

    qmlRegisterSingletonType<ImageUtils>("ImageUtils", 1, 0, "ImageUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        ImageUtils *utils = new ImageUtils();
        return utils;
    });

    qmlRegisterSingletonType<FileUtils>("SettingsUtils", 1, 0, "SettingsUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        SettingsUtils *utils = new SettingsUtils();
        return utils;
    });

    qmlRegisterSingletonType<Clipboard>("Clipboard", 1, 0, "Clipboard", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        auto *clipboard = new Clipboard();
        return clipboard;
    });

    QQuickStyle::setStyle("Material");
}

void Setup::qmlEngineAvailable(QQmlEngine *engine)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    engine->addImportPath("qrc:/qt5");
    #if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
        engine->addImportPath("qrc:/qt512");
    #endif //
#else
    engine->addImportPath("qrc:/qt6");
#endif //

#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0) || QT_VERSION == QT_VERSION_CHECK(5, 13, 0)
    engine->addImportPath("qrc:/qt513+");
#endif //

    qDebug() << "qmlEngineAvailable";
}
