import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0
import QtQuick.Controls 1.4 as QQC1
import Qt.labs.platform 1.1

ApplicationWindow {
    id: root
    width: 1600
    height: 1280
    visible: true
    title: qsTr("DVRescue")
    color: "#2e3436"

    DvRescueLogo {
        id: dvRescueLogo
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
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
            buttons: [transferButton, analysisButton, packageButton]
        }

        NavButton {
            id: transferButton
            // text: qsTr("Transfer")
            checkable: true;
            property int index: 0
            icon.source: "icons/menu-transfer.svg"
        }
        NavButton {
            id: analysisButton
            // text: qsTr("Analysis")
            checkable: true;
            property int index: 1
            checked: true
            icon.source: "icons/menu-analysis.svg"
        }
        NavButton {
            id: packageButton
            // text: qsTr("Package")
            checkable: true;
            property int index: 2
            icon.source: "icons/menu-package.svg"
        }
        NavButton {
            // text: qsTr("Settings")
            onClicked: {
                toolsDialog.show();
            }
            icon.source: "icons/menu-settings.svg"
        }
        NavButton {
            // text: qsTr("Help")
            icon.source: "icons/menu-help.svg"
        }
        NavButton {
            // text: qsTr("Debug")
            onClicked: {
                debugView.visible = !debugView.visible
            }
            icon.source: "icons/menu-debug.svg"
        }
    }

    DvRescueReport {
        id: report
    }

    ListModel {
        id: filesModel

        signal appended(var fileInfo)
        signal removed(int index, var fileInfo)

        function add(filePath) {
            console.debug('add: ', filePath)

            var fileInfo = report.resolveRelatedInfo(filePath)
            append(fileInfo)

            console.debug('emitting appended: ', JSON.stringify(fileInfo, 0, 4))
            appended(fileInfo)
        }

        function del(index) {
            console.debug('delete: ', index)

            var fileInfo = get(index);
            remove(index);

            console.debug('emitting removed: ', index)
            removed(index, fileInfo)
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

        TransferPage {
            id: mainUI

            property var urlToPath: function(url) {
                return FileUtils.getFilePath(url);
            }

            queryStatusCallback: () => {
                return avfctl.status(0)
            }

            rewindButton.onClicked: {
                pendingAction = true;
                statusText = "rewinding..";
                avfctl.rew(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   statusText = "rewinding.";
                   pendingAction = false;
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            stopButton.onClicked: {
                pendingAction = true;
                statusText = "stopping..";
                avfctl.stop(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   statusText = "stopping.";
                   pendingAction = false;
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            playButton.onClicked: {
                pendingAction = true;
                statusText = "playing..";
                avfctl.play(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   statusText = "playing.";
                   pendingAction = false;
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            fastForwardButton.onClicked: {
                pendingAction = true;
                statusText = "fast-forwarding..";
                avfctl.ff(0,  (launcher) => {
                    commandsLogs.logCommand(launcher);
                }).then((result) => {
                    statusText = "fast-forwarding.";
                    pendingAction = false;
                    commandsLogs.logResult(result.outputText);
                    return result;
                });
            }

            captureButton.onClicked: {
                specifyPathDialog.callback = (fileUrl) => {
                    var filePath = urlToPath(fileUrl);

                    pendingAction = true;
                    dvrescue.grab(0, filePath, (launcher) => {
                       launcher.errorChanged.connect((errorBytes) => {
                           console.debug('grabbed errorString: ', errorBytes)
                           var errorString = '' + errorBytes;
                           var splitted = errorString.trim().split('\r');
                           statusText = splitted[splitted.length - 1]
                       });

                       console.debug('logging grab command')
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                       commandsLogs.logResult(result.outputText);
                       return result;
                    }).catch((e) => {
                       commandsLogs.logResult(e);
                    });
                }

                specifyPathDialog.open();
            }

            deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(0).name + " (" + devicesModel.get(0).type + ")"
        }

        AnalysePage {
            id: analysePage
            filesModel: filesModel
            recentFilesModel: recentFilesModel
        }

        PackagePage {
            id: packagePage
            filesModel: filesModel
            recentFilesModel: recentFilesModel

            dvrescueCmd: settings.dvrescueCmd
            xmlStarletCmd: settings.xmlStarletCmd
            mediaInfoCmd: settings.mediaInfoCmd
            ffmpegCmd: settings.ffmpegCmd
        }
    }

    AvfCtl {
        id: avfctl
        cmd: settings.avfctlCmd

        Component.onCompleted: {
            console.debug('avfctl completed...');
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

    PathResolver {
        id: pathResolver
    }

    Settings {
        id: settings;
        property string avfctlCmd
        property string dvrescueCmd
        onDvrescueCmdChanged: {
            console.debug('dvrescueCmd = ', dvrescueCmd)
        }

        property string xmlStarletCmd
        property string mediaInfoCmd
        property string ffmpegCmd
        property alias recentFilesJSON: recentFilesModel.recentFilesJSON

        Component.onCompleted: {
            console.debug('settings initialized')
        }
    }

    ToolsDialog {
        id: toolsDialog

        onReset: {
            avfctlCmd = pathResolver.resolve("avfctl")
            dvrescueCmd = pathResolver.resolve("dvrescue")
            ffmpegCmd = pathResolver.resolve("ffmpeg")
            mediaInfoCmd = pathResolver.resolve("mediainfo")
            xmlStarletCmd = pathResolver.resolve(Qt.platform.os === "windows" ? "xml" : "xmlstarlet")
        }

        onAccepted: {
            settings.avfctlCmd = avfctlCmd
            settings.dvrescueCmd = dvrescueCmd
            settings.ffmpegCmd = ffmpegCmd
            settings.mediaInfoCmd = mediaInfoCmd
            settings.xmlStarletCmd = xmlStarletCmd
        }

        function show() {
            avfctlCmd = settings.avfctlCmd
            dvrescueCmd = settings.dvrescueCmd
            xmlStarletCmd = settings.xmlStarletCmd
            mediaInfoCmd = settings.mediaInfoCmd
            ffmpegCmd = settings.ffmpegCmd

            open();
        }
    }

    SelectPathDialog {
        id: selectPathDialog
    }

    SpecifyPathDialog {
        id: specifyPathDialog
        nameFilters: ["Video files (*.dv)"]
    }

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        getDevices: function() {
            return avfctl.queryDecks((launcher) => {
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

        if(!toolsDialog.validateTool(settings.avfctlCmd))
            settings.avfctlCmd = pathResolver.resolve("avfctl")
        if(!toolsDialog.validateTool(settings.dvrescueCmd))
            settings.dvrescueCmd = pathResolver.resolve("dvrescue")
        if(!toolsDialog.validateTool(settings.ffmpegCmd))
            settings.ffmpegCmd = pathResolver.resolve("ffmpeg")
        if(!toolsDialog.validateTool(settings.mediaInfoCmd))
            settings.mediaInfoCmd = pathResolver.resolve("mediainfo")
        if(!toolsDialog.validateTool(settings.xmlStarletCmd))
            settings.xmlStarletCmd = pathResolver.resolve(Qt.platform.os === "windows" ? "xml" : "xmlstarlet")

        console.debug('checking tools...')
        if(!toolsDialog.areToolsSpecified([settings.avfctlCmd, settings.dvrescueCmd, settings.ffmpegCmd,
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
