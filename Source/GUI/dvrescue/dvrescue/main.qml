import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    AvfCtl {
        id: avfctl
    }

    Settings {
        property alias avfctlCmd: avfctl.avfctlCmd
    }

    RowLayout {
        id: selectAvfctlRow
        anchors.left: parent.left
        anchors.right: parent.right

        TextField {
            id: input

            Layout.fillWidth: true
            onAccepted: {
                avfctl.avfctlCmd = text
            }

            Component.onCompleted: {
                input.text = avfctl.avfctlCmd
            }
        }

        Button {
            text: "Apply"
            onClicked: {
                avfctl.avfctlCmd = input.text
            }
        }
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

    DevicesView {
        id: devicesView
        model: devicesModel
        anchors.top: selectAvfctlRow.bottom
        onPlayClicked: {
            avfctl.play(index, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }
        onStopClicked: {
            avfctl.stop(index, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }
        onMoveToStartClicked: {
            avfctl.rew(index, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }
        onMoveToEndClicked: {
            avfctl.ff(index,  (launcher) => {
                commandsLogs.logCommand(launcher);
            }).then((result) => {
                commandsLogs.logResult(result.outputText);
                return result;
            });
        }
    }

    Item {
        anchors.left: devicesView.right
        anchors.leftMargin: 100
        anchors.top: selectAvfctlRow.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        TabBar {
            id: tabBar
            width: parent.width

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
                        append(launcher.program() + ' ' + launcher.arguments().join(' '));
                        append('\n');
                    }

                    function logResult(result) {
                        append(result);
                        append('\n\n');
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
                }
            }
        }
    }
}
