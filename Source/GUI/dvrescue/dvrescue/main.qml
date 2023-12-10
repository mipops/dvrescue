import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0
import Qt.labs.platform 1.1 as Platform

ApplicationWindow {
    id: root
    width: 1600
    height: 1280
    title: qsTr("DVRescue")
    color: "#2e3436"

    Platform.MenuBar {
        Platform.Menu {
            title: "dvrescue"

            Platform.MenuItem {
                text: "About dvrescue"
                onTriggered: {
                    about.open()
                }
            }

            Platform.MenuItem {
                text: "Preferences"
                onTriggered: {
                    toolsDialog.show()
                }
            }
        }
    }

    DvRescueLogo {
        id: dvRescueLogo
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10

        TapHandler {
            onTapped: {
                about.open()
            }
        }
    }

    AboutDialog {
        id: about

        version: buildVersionString
        buildDate: buildDateString
        buildQtVersion: buildQtVersionString
        runtimeQtVersion: runtimeQtVersionString
        buildFFmpegVersion: buildFFmpegVersionString
        runtimeFFmpegVersion: runtimeFFmpegVersionString
    }

    Column {
        id: navigationColumn
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: dvRescueLogo.bottom
        anchors.topMargin: 10
        spacing: 10

        ButtonGroup {
            id: navigationButtons
            buttons: [captureButton, analysisButton, mergeButton, packageButton]
            onClicked: {
                settings.defaultTabIndex = button.index
            }
        }

        NavButton {
            id: captureButton
            // text: qsTr("Capture")
            checkable: true;
            property int index: 0
            icon.source: "/icons/menu-capture.svg"
        }
        NavButton {
            id: analysisButton
            // text: qsTr("Analysis")
            checkable: true;
            property int index: 1
            checked: true
            onCheckedChanged: {
                if(!checked) {
                    analysePage.playerView.player.stop();
                }
            }

            icon.source: "/icons/menu-analysis.svg"
        }
        NavButton {
            id: mergeButton
            // text: qsTr("Merge")
            checkable: true;
            property int index: 2
            icon.source: "/icons/menu-merge.svg"
        }
        NavButton {
            id: packageButton
            // text: qsTr("Package")
            checkable: true;
            property int index: 3
            icon.source: "/icons/menu-package.svg"
        }
        NavButton {
            // text: qsTr("Settings")
            onClicked: {
                toolsDialog.show();
            }
            icon.source: "/icons/menu-settings.svg"
        }
        NavButton {
            // text: qsTr("Help")
            onClicked: {
                Qt.openUrlExternally("https://mipops.github.io/dvrescue/");
            }
            icon.source: "/icons/menu-help.svg"
        }
        NavButton {
            // text: qsTr("Debug")
            onClicked: {
                debugView.visible = !debugView.visible
            }
            icon.source: "/icons/menu-debug.svg"
            visible: settings.debugVisible
        }

        Component.onCompleted: {
            navigationButtons.buttons[settings.defaultTabIndex].checked = true
        }
    }

    DvRescueReport {
        id: report
    }

    FilesModel {
        id: filesModel
        mediaInfoModel: instantiator

        function parseReport(reportPath, index) {
            var mediaInfo = instantiator.objectAt(index)
            mediaInfo.reportPath = reportPath;
            mediaInfo.resolve();
        }

        function makeReport(fileInfo, index) {

            Qt.callLater(() => {
                             var mediaInfo = instantiator.objectAt(index)
                             mediaInfo.editRow(index, 'Progress', -1)
                         })

            dvrescue.makeReport(fileInfo.originalPath, (launcher) => {
                debugView.logCommand(launcher);
            }).then((reportPath) => {
                console.debug('resolved report path: ', reportPath)
                filesModel.setProperty(index, 'reportPath', reportPath)

                var mediaInfo = instantiator.objectAt(index)
                mediaInfo.editRow(index, 'Progress', 0)

                parseReport(reportPath, index)
            })
        }

        onAppended: {
            var index = filesModel.count - 1
            console.debug('FilesModel: onAppended', index, JSON.stringify(fileInfo))

            if(fileInfo.reportPath === '') {
                makeReport(fileInfo, index);
            } else {
                parseReport(fileInfo.reportPath, index)
            }
        }
    }

    MediaInfoModel {
        id: instantiator
        model: filesModel
        onObjectAdded: {
            console.debug('MediaInfoModel: added', index, JSON.stringify(object))
        }
        onObjectRemoved: {
            console.debug('MediaInfoModel: removed', index, JSON.stringify(object))
        }
    }

    ListModel {
        id: recentFilesModel

        signal selected(string filePath);

        property string recentFilesJSON : ''
        property var recentFiles: recentFilesJSON === '' ? [] : JSON.parse(recentFilesJSON)

        function toArray() {
            var recentFiles = []
            for(var i = 0; i < count; ++i) {
                recentFiles.push(get(i).filePath);
            }
            return recentFiles;
        }

        function addRecent(filePath) {

            var newRecentFiles = recentFiles.filter(item => item !== filePath)
            newRecentFiles.unshift(filePath)

            if(newRecentFiles.length > 10) {
                newRecentFiles.pop();
            }
            recentFiles = newRecentFiles
            recentFilesJSON = JSON.stringify(recentFiles)

            clear();
            for(var i = 0; i < newRecentFiles.length; ++i) {
                append({'filePath' : newRecentFiles[i]})
            }

            console.debug('recentFilesJSON: ', JSON.stringify(recentFilesJSON, 0, 4))
        }

        Component.onCompleted: {
            var recentFiles = recentFilesJSON === '' ? [] : JSON.parse(recentFilesJSON)
            recentFiles.forEach((filePath) => {
                append({'filePath' : filePath})
            })
        }
    }

    StackLayout {
        anchors.left: navigationColumn.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        currentIndex: navigationButtons.checkedButton.index

        CapturePage {
            id: mainUI

            property var urlToPath: function(url) {
                return FileUtils.getFilePath(url);
            }

            onGrabCompleted: {
                filesModel.add(filePath)
            }

            onCommandExecutionStarted: debugView.logCommand(launcher)
            onCommandExecutionFinished: debugView.logResult(results)
        }

        AnalysePage {
            id: analysePage
            filesModel: filesModel
            recentFilesModel: recentFilesModel

            onCommandExecutionStarted: debugView.logCommand(launcher)
            onCommandExecutionFinished: debugView.logResult(results)
        }

        MergePage {
            id: mergePage
            filesModel: filesModel
            recentFilesModel: recentFilesModel

            dvrescueCmd: settings.dvrescueCmd
            xmlStarletCmd: settings.xmlStarletCmd
            mediaInfoCmd: settings.mediaInfoCmd
            ffmpegCmd: settings.ffmpegCmd

            onCommandExecutionStarted: debugView.logCommand(launcher)
            onCommandExecutionFinished: debugView.logResult(results)
        }

        PackagePage {
            id: packagePage
            filesModel: filesModel
            recentFilesModel: recentFilesModel
            framesCount: analysePage.framesCount

            dvrescueCmd: settings.dvrescueCmd
            xmlStarletCmd: settings.xmlStarletCmd
            mediaInfoCmd: settings.mediaInfoCmd
            ffmpegCmd: settings.ffmpegCmd

            onCommandExecutionStarted: debugView.logCommand(launcher)
            onCommandExecutionFinished: debugView.logResult(results)
        }
    }

    DvRescueCLI {
        id: dvrescue
        cmd: settings.dvrescueCmd

        Component.onCompleted: {
            console.debug('dvrescuecli completed...');
        }
    }

    DvPackagerCtl {
        id: packagerCtl

        dvrescueCmd: settings.dvrescueCmd
        xmlStarletCmd: settings.xmlStarletCmd
        mediaInfoCmd: settings.mediaInfoCmd
        ffmpegCmd: settings.ffmpegCmd

        Component.onCompleted: {
            if(Qt.platform.os === "windows") {
                packagerCtl.paths = [ FileUtils.getFileDir(settings.dvrescueCmd), FileUtils.getFileDir(settings.xmlStarletCmd),
                                      FileUtils.getFileDir(settings.mediaInfoCmd), FileUtils.getFileDir(settings.ffmpegCmd) ]
            }
        }
    }

    DvPlayCtl {
        id: dvplay

        xmlStarletCmd: settings.xmlStarletCmd
        mediaInfoCmd: settings.mediaInfoCmd
        ffmpegCmd: settings.ffmpegCmd

        Component.onCompleted: {
            if(Qt.platform.os === "windows") {
                paths = [ FileUtils.getFileDir(settings.dvrescueCmd), FileUtils.getFileDir(settings.xmlStarletCmd),
                                      FileUtils.getFileDir(settings.mediaInfoCmd), FileUtils.getFileDir(settings.ffmpegCmd) ]
            }
        }
    }

    PathResolver {
        id: pathResolver
    }

    Settings {
        id: settings;

        property bool keepFramesAtNonStandardPlaybackSpeed
        property bool keepFramesThatAllFullyConcealed
        property string endTheCaptureIftheTapeContainsNoDataFor
        property string retryToReadFramesWithErrorsUpTo
        property bool saveALogOfTheCaptureProcess

        property bool advancedFrameTable
        property var frameTableColumns: []

        property string dvrescueCmd
        property string xmlStarletCmd
        property string mediaInfoCmd
        property string ffmpegCmd
        property alias recentFilesJSON: recentFilesModel.recentFilesJSON
        property bool debugVisible
        property int defaultTabIndex

        Component.onCompleted: {
            console.debug('settings initialized')
        }
    }

    ToolsDialog {
        id: toolsDialog
        simpleFrameTableColumns: analysePage.dataView.model.simpleColumnsNames

        onReset: {
            console.debug('onReset: currentIndex = ', currentIndex)

            if(currentIndex === 0) {
                ignoreFramesAtNonStandardPlaybackSpeed = true
                keepFramesThatAllFullyConcealed = true
                endTheCaptureIftheTapeContainsNoDataFor = ''
                retryToReadFramesWithErrorsUpTo = ''
                notSaveALogOfTheCaptureProcess = true
            } else if(currentIndex === 1) {
                simpleFrameTable = true
                selectedFrameTableColumns.forEach((c) => {
                                                    c.selected = true;
                                                  })
                selectedFrameTableColumns = JSON.parse(JSON.stringify(selectedFrameTableColumns))
            } else {
                dvrescueCmd = pathResolver.resolve("dvrescue")
                ffmpegCmd = pathResolver.resolve("ffmpeg")
                mediaInfoCmd = pathResolver.resolve("mediainfo")
                xmlStarletCmd = pathResolver.resolve(Qt.platform.os === "windows" ? "xml" : "xmlstarlet")
                enableDebugView = false
            }

            var keys = SettingsUtils.keys();
            for(var i = 0; i < keys.length; ++i) {
                var key = keys[i]
                console.debug('setting key: ', key, 'value: ', settings.value(key))
            }
        }

        onAccepted: {
            console.debug('onAccepted')

            console.debug('keepFramesAtNonStandardPlaybackSpeed = ', keepFramesAtNonStandardPlaybackSpeed)
            console.debug('keepFramesThatAllFullyConcealed = ', keepFramesThatAllFullyConcealed)

            settings.keepFramesAtNonStandardPlaybackSpeed = keepFramesAtNonStandardPlaybackSpeed
            settings.keepFramesThatAllFullyConcealed = keepFramesThatAllFullyConcealed
            settings.endTheCaptureIftheTapeContainsNoDataFor = endTheCaptureIftheTapeContainsNoDataFor
            settings.retryToReadFramesWithErrorsUpTo = retryToReadFramesWithErrorsUpTo
            settings.saveALogOfTheCaptureProcess = saveALogOfTheCaptureProcess
            settings.advancedFrameTable = advancedFrameTable

            var filteredSelectedFrameTableColumns = selectedFrameTableColumns.filter((column) => { return column.selected })
            var mappedSelectedFrameTableColumns = filteredSelectedFrameTableColumns.map((column) => { return column.name });

            console.debug('selectedFrameTableColumns: ', JSON.stringify(selectedFrameTableColumns, 0, 4))
            console.debug('filtered selectedFrameTableColumns: ', JSON.stringify(filteredSelectedFrameTableColumns, 0, 4))
            console.debug('mapped selectedFrameTableColumns: ', JSON.stringify(mappedSelectedFrameTableColumns, 0, 4))

            settings.frameTableColumns = mappedSelectedFrameTableColumns

            settings.dvrescueCmd = dvrescueCmd
            settings.ffmpegCmd = ffmpegCmd
            settings.mediaInfoCmd = mediaInfoCmd
            settings.xmlStarletCmd = xmlStarletCmd
            settings.debugVisible = enableDebugView

            var keys = SettingsUtils.keys();
            for(var i = 0; i < keys.length; ++i) {
                var key = keys[i]
                console.debug('setting key: ', key, 'value: ', settings.value(key))
            }
        }

        function show() {
            console.debug('show')
            var keys = SettingsUtils.keys();
            for(var i = 0; i < keys.length; ++i) {
                var key = keys[i]
                console.debug('setting key: ', key, 'value: ', settings.value(key))
            }

            if(settings.keepFramesAtNonStandardPlaybackSpeed)
                keepFramesAtNonStandardPlaybackSpeed = true
            else
                ignoreFramesAtNonStandardPlaybackSpeed = true

            if(settings.keepFramesThatAllFullyConcealed)
                keepFramesThatAllFullyConcealed = true
            else
                ignoreFramesThatAllFullyConcealed = true

            endTheCaptureIftheTapeContainsNoDataFor = settings.endTheCaptureIftheTapeContainsNoDataFor
            retryToReadFramesWithErrorsUpTo = settings.retryToReadFramesWithErrorsUpTo

            if(settings.saveALogOfTheCaptureProcess)
                saveALogOfTheCaptureProcess = true
            else
                notSaveALogOfTheCaptureProcess = true

            dvrescueCmd = settings.dvrescueCmd
            xmlStarletCmd = settings.xmlStarletCmd
            mediaInfoCmd = settings.mediaInfoCmd
            ffmpegCmd = settings.ffmpegCmd
            enableDebugView = settings.debugVisible

            if(settings.advancedFrameTable) {
                advancedFrameTable = true
            } else {
                simpleFrameTable = true
            }

            var initialSelectedFrameTableColumns = []
            console.debug('settings.frameTableColumns: ', JSON.stringify(settings.frameTableColumns, 0, 4))

            analysePage.dataView.model.columnsNames.forEach((c) => {
                                        var selected = settings.frameTableColumns.indexOf(c) !== -1;
                                        initialSelectedFrameTableColumns.push({'name' : c, 'selected' : selected})
                                })
            selectedFrameTableColumns = initialSelectedFrameTableColumns

            open();
        }
    }

    SelectPathDialog {
        id: selectPathDialog
    }

    SpecifyPathDialog {
        id: specifyPathDialog
        filePath: Platform.StandardPaths.writableLocation(Platform.StandardPaths.MoviesLocation)
        fileName: 'out' + extension
    }

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        getDevices: function() {
            return dvrescue.queryDecks((launcher) => {
                statusLogs.logCommand(launcher);
            }).then((result) => {
                statusLogs.logResult(result.outputText);
                return result;
            });
        }
    }

    Window {
        id: debugView
        width: root.width / 2
        height: root.height

        function logCommand(launcher) {
            commandsLogs.logCommand(launcher)
        }

        function logResult(result) {
            commandsLogs.logResult(result)
        }

        Component.onCompleted: {
            x = root.width
            y = root.y
        }

        Rectangle {
            anchors.fill: parent
            color: "#ccffffff"

            TabBar {
                id: tabBar
                width: parent.width
                currentIndex: 0

                TabButton {
                    text: "command logs"
                }
                TabButton {
                    text: "status logs"
                }
            }

            StackLayout {
                id: stack
                anchors.top: tabBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                currentIndex: tabBar.currentIndex

                ScrollView {
                    TextArea {
                        id: commandsLogs
                        selectByMouse: true
                        wrapMode: TextEdit.WrapAnywhere

                        function logCommand(launcher) {
                            console.debug('logging command: ', launcher.program() + ' ' + launcher.arguments().join(' '))

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning commands log");
                        }
                    }
                }

                ScrollView {
                    TextArea {
                        id: statusLogs
                        selectByMouse: true

                        function logCommand(launcher) {
                            if(text.length > 50000)
                                clear();

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning status log");
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        console.debug('main.qml completed')

        var keys = SettingsUtils.keys();
        for(var i = 0; i < keys.length; ++i) {
            var key = keys[i]
            console.debug('setting key: ', key, 'value: ', settings.value(key))
        }

        if(!toolsDialog.validateTool(settings.dvrescueCmd))
            settings.dvrescueCmd = pathResolver.resolve("dvrescue")
        if(!toolsDialog.validateTool(settings.ffmpegCmd))
            settings.ffmpegCmd = pathResolver.resolve("ffmpeg")
        if(!toolsDialog.validateTool(settings.mediaInfoCmd))
            settings.mediaInfoCmd = pathResolver.resolve("mediainfo")
        if(!toolsDialog.validateTool(settings.xmlStarletCmd))
            settings.xmlStarletCmd = pathResolver.resolve(Qt.platform.os === "windows" ? "xml" : "xmlstarlet")

        console.debug('checking tools...')
        if(!toolsDialog.areToolsSpecified([settings.dvrescueCmd, settings.ffmpegCmd,
                                           settings.mediaInfoCmd, settings.xmlStarletCmd]))
        {
            toolsDialog.show()
        }
    }

    Component.onDestruction: {
        console.debug('main.qml destructed');

        var keys = SettingsUtils.keys();
        for(var i = 0; i < keys.length; ++i) {
            var key = keys[i]
            console.debug('setting key: ', key, 'value: ', settings.value(key))
        }
    }
}
