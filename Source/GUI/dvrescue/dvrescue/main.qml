import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import GraphModel 1.0
import QwtQuick2 1.0

Window {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Hello World")

    GraphModel {
        id: graphModel
        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();
            graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
        }
    }

    Rectangle {
        anchors.fill: parent;
        z: 100

        RowLayout {
            id: toolsLayout
            TextField {
                id: xmlPath
            }

            Button {
                text: "select file"
                onClicked: {
                    selectPathDialog.callback = (fileUrl) => {
                        var filePath = FileUtils.getFilePath(fileUrl);
                        xmlPath.text = filePath;
                    }
                    selectPathDialog.open();
                }
            }

            Button {
                text: "load"
                onClicked: {
                    refreshTimer.start();
                    graphModel.populate(xmlPath.text);
                }
            }
        }

        QwtQuick2Plot {
            id: videoPlot
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.top: toolsLayout.bottom
            height: (parent.height - toolsLayout.height) / 2 - anchors.topMargin

            QwtQuick2PlotCurve {
                id: videoCurve
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "green"
            }

            QwtQuick2PlotCurve {
                id: videoCurve2
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "red"
            }

            QwtQuick2PlotGrid {
                enableXMin: true
                enableYMin: true
                majorPenColor: Qt.darkGray
                majorPenStyle: Qt.DotLine
                minorPenColor: Qt.gray
                minorPenStyle: Qt.DotLine
            }
        }

        QwtQuick2Plot {
            id: audioPlot
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.top: videoPlot.bottom
            height: videoPlot.height

            QwtQuick2PlotCurve {
                id: audioCurve
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "blue"
            }

            QwtQuick2PlotCurve {
                id: audioCurve2
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "yellow"
            }

            QwtQuick2PlotGrid {
                enableXMin: true
                enableYMin: true
                majorPenColor: Qt.darkGray
                majorPenStyle: Qt.DotLine
                minorPenColor: Qt.gray
                minorPenStyle: Qt.DotLine
            }
        }

        Timer {
            id: refreshTimer
            interval: 500
            running: true
            onTriggered: {
                console.debug('updating plots...')
                graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
            }
        }
    }

    AvfCtl {
        id: avfctl
    }

    Settings {
        id: settings;
        property alias avfctlCmd: avfctl.avfctlCmd
        property alias selectedXml: xmlPath.text
    }

    Dialog {
        id: avfctlDialog
        title: "Please, specify avfctl tool location"
        contentWidth: 480

        TextField {
            id: avfctlField
            width: 480

            placeholderText: "avfctl tool path..."
            selectByMouse: true
        }

        standardButtons: Dialog.Cancel | Dialog.Ok
        onAccepted: {
            avfctl.avfctlCmd = avfctlField.text
        }
        anchors.centerIn: parent
    }

    SelectPathDialog {
        id: selectPathDialog
    }

    SpecifyPathDialog {
        id: specifyPathDialog
    }

    SimpleTransfer {
        width: 1280
        height: 720

        property var urlToPath: function(url) {
            return FileUtils.getFilePath(url);
        }

        rewindMouseArea.onClicked: {
            avfctl.rew(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        stopMouseArea.onClicked: {
            avfctl.stop(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        playMouseArea.onClicked: {
            avfctl.play(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        fastForwardMouseArea.onClicked: {
            avfctl.ff(0,  (launcher) => {
                commandsLogs.logCommand(launcher);
            }).then((result) => {
                commandsLogs.logResult(result.outputText);
                return result;
            });
        }

        grabMouseArea.onClicked: {
            specifyPathDialog.callback = (fileUrl) => {
                var filePath = urlToPath(fileUrl);

                avfctl.grab(0, filePath, (launcher) => {
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

        settingsMouseArea.onClicked: {
            avfctlField.text = avfctl.avfctlCmd
            avfctlField.forceActiveFocus();
            avfctlDialog.open();
        }

        deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(0).name + " (" + devicesModel.get(0).type + ")"
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

    Rectangle {
        id: debugRect
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: 162
        height: 100
        color: "#a40000"

        Text {
            anchors.centerIn: parent
            color: "#ffffff"
            text: qsTr("Debug")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }
    }

    MouseArea {
        anchors.fill: debugRect
        onClicked: {
            debugView.visible = !debugView.visible
        }
    }

    Window {
        id: debugView
        visible: false
        width: root.width / 2
        height: root.height

        Component.onCompleted: {
            x = root.width
            y = root.y
        }

        Rectangle {
            width: parent.width / 2
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
                anchors.top: tabBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                currentIndex: tabBar.currentIndex

                ScrollView {
                    TextArea {
                        id: commandsLogs
                        selectByMouse: true

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
}
