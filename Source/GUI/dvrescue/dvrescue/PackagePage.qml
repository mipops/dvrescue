import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1
import QtQuick.Controls 1.4 as QQC1

Item {
    id: root

    property string dvrescueCmd
    property string xmlStarletCmd
    property string mediaInfoCmd
    property string ffmpegCmd
    property alias filesModel: fileView.filesModel
    property alias recentFilesModel: recentsPopup.filesModel
    property int framesCount

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

    RecentsPopup {
        id: recentsPopup
        onSelected: {
            root.filesModel.add(filePath)
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

        anchors.left: parent.left
        anchors.right: parent.right

        property string dvRescueXmlExtension: ".dvrescue.xml"
        property int fileViewerHeight: 0

        CustomButton {
            id: addFiles
            icon.source: "icons/add-files.svg"
            onClicked: {
                selectPath.callback = (urls) => {
                    urls.forEach((url) => {
                                     var filePath = FileUtils.getFilePath(url);
                                     filesModel.add(filePath);
                                     root.recentFilesModel.addRecent(filePath)
                                 });
                }

                selectPath.open();
            }
        }

        CustomButton {
            icon.source: "icons/recent.svg"

            onClicked: {
                var mapped = mapToItem(root, 0, 0);
                recentsPopup.x = mapped.x
                recentsPopup.y = mapped.y + height

                recentsPopup.open();
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: addFiles.height
            color: 'white'

            RowLayout {
                id: settingsRow
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

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
                    Layout.minimumWidth: 400
                    placeholderText: "additional options..."
                }

                Button {
                    text: "Package"
                    height: parent.height
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
        }
    }


    QQC1.SplitView {
        id: splitView
        anchors.top: toolsLayout.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        orientation: Qt.Vertical

        PackageFileView {
            id: fileView
            height: parent.height / 1.5

            onFileAdded: {
                root.recentFilesModel.addRecent(filePath)
            }
        }

        SegmentDataViewWithToolbar {
            id: segmentDataView
            height: parent.height / 2.5
            framesCount: framesCount
            reportPath: fileView.currentIndex !== -1 ? fileView.mediaInfoAt(fileView.currentIndex).reportPath : null
            onReportPathChanged: {
                populateSegmentData()
            }
        }

        Rectangle {
            color: 'red'
        }
    }
}
