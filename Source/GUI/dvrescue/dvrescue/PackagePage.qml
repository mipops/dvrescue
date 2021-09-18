import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1

Item {
    id: root

    property string dvrescueCmd
    property string xmlStarletCmd
    property string mediaInfoCmd
    property string ffmpegCmd

    DropArea {
        id: dropArea;
        anchors.fill: parent
        onEntered: {
            drag.accept (Qt.LinkAction);
        }
        onDropped: {
            if(drop.urls.length !== 0)
            {
                drop.urls.forEach((url) => {
                                      var filePath = FileUtils.getFilePath(url);
                                      var entries = FileUtils.ls(FileUtils.getFileDir(filePath));
                                      fileView.add(filePath)
                                  })
            }

            console.log(drop.urls)
        }
        onExited: {
        }
    }

    Rectangle {
        z: 100
        color: 'darkgray'
        opacity: 0.5
        anchors.fill: parent
        visible: busy.running

        MouseArea {
            anchors.fill: parent
        }
    }

    BusyIndicator {
        id: busy
        z: 100
        width: Math.min(parent.width, parent.height) / 3
        height: width;
        visible: running
        running: false
        anchors.centerIn: parent
    }

    property string recentFilesJSON : ''
    property var recentFiles: recentFilesJSON === '' ? [] : JSON.parse(recentFilesJSON)
    onRecentFilesChanged: {
        console.debug('PackagePage: recentFiles = ', JSON.stringify(recentFiles, 0, 4))
    }

    function addRecent(filePath) {
        var newRecentFiles = recentFiles.filter(item => item !== filePath)
        newRecentFiles.unshift(filePath)

        if(newRecentFiles.length > 10) {
            newRecentFiles.pop();
        }
        recentFiles = newRecentFiles
        recentFilesJSON = JSON.stringify(recentFiles)
    }

    PackageFileView {
        id: fileView
        height: parent.height / 2
        anchors.left: parent.left
        anchors.right: parent.right

        onFileAdded: {
            addRecent(filePath)
        }
    }

    RecentsPopup {
        id: recentsPopup
        files: recentFiles
        onSelected: {
            fileView.add(filePath)
        }
    }

    SelectPathDialog {
        id: selectPath
        selectMultiple: true
        nameFilters: [
            "Video files (*.mov *.mkv *.avi *.dv *.mxf)"
        ]
    }

    RowLayout {
        id: toolsLayout
        anchors.top: fileView.bottom
        Layout.fillWidth: true

        property string dvRescueXmlExtension: ".dvrescue.xml"
        property int fileViewerHeight: 0

        CustomButton {
            icon.source: "icons/add-files.svg"
            onClicked: {
                selectPath.callback = (urls) => {
                    urls.forEach((url) => {
                                     fileView.add(FileUtils.getFilePath(url));
                                 });
                }

                selectPath.open();
            }
        }

        CustomButton {
            icon.source: "icons/recent.svg"

            onClicked: {
                var mapped = mapToItem(root, 0, 0);
                recentsPopup.x = mapped.x - recentsPopup.width + width
                recentsPopup.y = mapped.y + height

                recentsPopup.open();
            }
        }
    }

    Rectangle {
        color: 'white'
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: toolsLayout.bottom
        anchors.bottom: parent.bottom
    }

    Row {
        id: settingsRow
        anchors.top: toolsLayout.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        CheckBox {
            id: s
            text: "-s"
        }
        CheckBox {
            id: d
            text: "-d"
        }
        CheckBox {
            id: t
            text: "-t"
        }
        CheckBox {
            id: sBig
            text: "-S"
        }

        TextField {
            id: additional
            width: 400
            placeholderText: "additional options..."
        }
    }


    Button {
        anchors.top: settingsRow.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "go"
        onClicked: {
            var path = fileView.files[Math.max(fileView.currentIndex, 0)];
            if(Qt.platform.os === "windows") {
                path = "/cygdrive/" + path.replace(":", "");
            }

            busy.running = true;
            var params = path;
            if(d.checked)
                params = "-d " + params;
            if(t.checked)
                params = "-t " + params;
            if(s.checked)
                params = "-s " + params;
            if(sBig.checked)
                params = "-S " + params;

            if(additional.text.length !== 0)
                params = additional.text + " " + params

            var extraParams = " -v -e mov -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
                .replace("{xml}", packagerCtl.effectiveXmlStarletCmd)
                .replace("{ffmpeg}", packagerCtl.effectiveFfmpegCmd)
                .replace("{dvrescue}", packagerCtl.effectiveDvrescueCmd)
                .replace("{mediainfo}", packagerCtl.effectiveMediaInfoCmd)

            packagerCtl.exec(params, (launcher) => {
                debugView.logCommand(launcher)
                launcher.outputChanged.connect((outputString) => {
                    debugView.logResult(outputString);
                })
            }, extraParams).then(() => {
                console.debug('executed....')
                busy.running = false;
            }).catch((error) => {
                debugView.logResult(error);
                busy.running = false;
            });
        }
    }
}
