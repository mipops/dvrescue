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

                Button {
                    text: "Package"
                    height: parent.height
                    onClicked: {
                        var videoPath = filesModel.get(fileView.currentIndex).videoPath;
                        var reportPath = filesModel.get(fileView.currentIndex).reportPath;

                        var promise = segmentDataViewWithToolbar.segmentDataView.packaging(reportPath, videoPath);
                        promise.then(() => {
                            console.debug('packaging done');
                        }).catch((err) => {
                            console.error('packaging failed: ', err);
                        })
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
            height: parent.height / 3

            onFileAdded: {
                root.recentFilesModel.addRecent(filePath)
            }
        }

        SegmentDataViewWithToolbar {
            id: segmentDataViewWithToolbar
            height: parent.height / 3
            framesCount: framesCount
            reportPath: fileView.currentIndex !== -1 ? fileView.mediaInfoAt(fileView.currentIndex).reportPath : null
            onReportPathChanged: {
                populateSegmentData()
            }

            onPopulated: {
                packageOutputFileView.dataModel.clear();
                for(var i = 0; i < segmentDataView.model.rowCount; ++i) {
                    packageOutputFileView.newRow(segmentDataView.model.getRow(i)['Segment #'])
                }
            }
        }

        PackageOutputFileView {
            id: packageOutputFileView
            height: parent.height / 3
        }
    }
}
