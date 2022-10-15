#include "fileutils.h"
#include "settingsutils.h"
#include "qtavplayerutils.h"
#include "launcher.h"
#include "thread.h"
#include "filewriter.h"
#include "csvparser.h"
#include "sortfiltertablemodel.h"
#include "qqmltablemodel_p.h"
#include "qqmltablemodelcolumn_p.h"
#include "mediaplayer.h"
#include "connectionutils.h"
#include "playbackbuffer.h"
#include <datamodel.h>
#include <mediainfo.h>
#include <logging.h>
#include <clipboard.h>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <qwtquick2plot.h>
#include <QQmlParserStatus>
#include <QQuickStyle>
#include <QFileInfo>
#include <imageutils.h>
#include <loggingutils.h>

extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/ffversion.h"
}
int main(int argc, char *argv[])
{
    LoggingUtils::installFilter();
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    qmlRegisterType<Launcher>("Launcher", 0, 1, "Launcher");
    qmlRegisterType<Thread>("Thread", 0, 1, "Thread");
    qmlRegisterType<FileWriter>("FileWriter", 0, 1, "FileWriter");
    qmlRegisterType<CsvParser>("CsvParser", 0, 1, "CsvParser");
    qmlRegisterType<DataModel>("DataModel", 1, 0, "DataModel");
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

    Logging logging;

    QApplication app(argc, argv);

#if defined(Q_OS_WIN)
    auto appDirPath = QCoreApplication::applicationDirPath();
    qDebug() << "appDirPath: " << appDirPath;
    auto paths = QProcessEnvironment::systemEnvironment().value("PATH");
    auto additionalPath =
            QDir::toNativeSeparators(appDirPath + "/" + "cygwin/bin") + ";" +
            QDir::toNativeSeparators(appDirPath + "/" + "scripts") + ";" +
            QDir::toNativeSeparators(appDirPath + "/" + "tools") + ";" +
            QDir::toNativeSeparators(appDirPath) + ";";

    paths.prepend(additionalPath);
    qputenv("PATH", paths.toUtf8());

    // Set TMPDIR to a known writable place for cygwin's mktemp
    auto tmpdir = QDir::tempPath();
    qputenv("TMPDIR", tmpdir.toUtf8());

#elif defined(Q_OS_MAC)
    auto appDirPath = QCoreApplication::applicationDirPath();
    qDebug() << "appDirPath: " << appDirPath;
    auto paths = QProcessEnvironment::systemEnvironment().value("PATH");
    auto additionalPath = QDir::toNativeSeparators(appDirPath + "/" + "../Helpers") + ":";
    paths.prepend(additionalPath);
    qputenv("PATH", paths.toUtf8());
#elif defined(Q_OS_LINUX)
    auto appDirPath = QCoreApplication::applicationDirPath();
    qDebug() << "appDirPath: " << appDirPath;
    auto prefix = QFileInfo(appDirPath).canonicalPath();
    qDebug() << "prefix: " << prefix;
    auto paths = QProcessEnvironment::systemEnvironment().value("PATH");
    auto additionalPath =
        QDir::toNativeSeparators(prefix + "/" + "lib64/dvrescue/bin") + ":" +
        QDir::toNativeSeparators(prefix + "/" + "lib/dvrescue/bin") + ":";

    paths.prepend(additionalPath);
    qputenv("PATH", paths.toUtf8());
#endif //

    app.setOrganizationName("dvrescue");
    app.setOrganizationDomain("dvrescue.com");
    app.setApplicationName("dvrescue");

    QCommandLineParser parser;
    parser.setApplicationDescription("dvrescue: GUI");

    QCommandLineOption resetSettingsOption(QStringList() << "r" << "resetsettings", "reset application settings");
    parser.addOption(resetSettingsOption);

    QCommandLineOption enableLoggingOption(QStringList() << "l" << "log", "enable file logging");
    parser.addOption(enableLoggingOption);

    QCommandLineOption loggingFilterRulesOption(QStringList() << "f" << "filter", "logging filter", "logging filter");
    parser.addOption(loggingFilterRulesOption);

    parser.process(app.arguments());
    if(parser.isSet(resetSettingsOption)) {
        qDebug() << "resetting settings...";
        QSettings settings;
        auto allKeys = settings.allKeys();
        for(auto& key : allKeys) {
            settings.remove(key);
        }
    }
    if(parser.isSet(enableLoggingOption)) {
        qDebug() << "enable file logging...";
        logging.enable();
    }

    QString filter;
    if(parser.isSet(loggingFilterRulesOption)) {
        filter = parser.value(loggingFilterRulesOption);
        qDebug() << "got logging filter rules: " << filter;
    }

    qDebug() << "PATH: " << qEnvironmentVariable("PATH");
    QQuickStyle::setStyle("Material");

    qmlRegisterSingletonType<Clipboard>("Clipboard", 1, 0, "Clipboard", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        auto *clipboard = new Clipboard();
        return clipboard;
    });

    qmlRegisterSingletonType<ConnectionUtils>("ConnectionUtils", 1, 0, "ConnectionUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        ConnectionUtils *utils = new ConnectionUtils();
        return utils;
    });

    qmlRegisterSingletonType<LoggingUtils>("LoggingUtils", 1, 0, "LoggingUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        LoggingUtils *utils = new LoggingUtils();
        return utils;
    });

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

    qmlRegisterSingletonType<QtAVPlayerUtils>("QtAVPlayerUtils", 1, 0, "QtAVPlayerUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        QtAVPlayerUtils *utils = new QtAVPlayerUtils();
        return utils;
    });

    QQmlApplicationEngine engine;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    engine.addImportPath("qrc:/qt5");
    #if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
        engine.addImportPath("qrc:/qt512");
    #endif //
#else
    engine.addImportPath("qrc:/qt6");
#endif //

#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0) || QT_VERSION == QT_VERSION_CHECK(5, 13, 0)
    engine.addImportPath("qrc:/qt513+");
#endif //

#ifndef BUILD_VERSION
#define BUILD_VERSION "devel"
#endif //

    engine.rootContext()->setContextProperty("buildVersionString", BUILD_VERSION);
    engine.rootContext()->setContextProperty("buildDateString", QString("%1 %2").arg(__DATE__).arg(__TIME__));

    engine.rootContext()->setContextProperty("buildQtVersionString", QString("Qt %1.%2.%3").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH));
    engine.rootContext()->setContextProperty("runtimeQtVersionString", qVersion());

    engine.rootContext()->setContextProperty("buildFFmpegVersionString", FFMPEG_VERSION);
    engine.rootContext()->setContextProperty("runtimeFFmpegVersionString", av_version_info());
    DataModel::setEngine(&engine);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    for(auto category : LoggingUtils::allCategories()) {
        qDebug() << "category: " << category->categoryName() << category->isDebugEnabled();
    }

    qDebug() << "\tapply logging filter rules: " << filter;

    if(!filter.isEmpty()) {
        LoggingUtils::uninstallFilter();
        QLoggingCategory::setFilterRules(filter);
    }

    // --filter "dvrescue.dvplay.debug=true"
    for(auto category : LoggingUtils::allCategories()) {
        qDebug() << "category: " << category->categoryName() << category->isDebugEnabled();
    }

    return app.exec();
}
