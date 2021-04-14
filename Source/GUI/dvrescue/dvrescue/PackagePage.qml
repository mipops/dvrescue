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
                                      // fileViewer.fileView.add(filePath)
                                      addRecent(filePath)
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

    property string bashName: Qt.platform.os === "windows" ? "bash.exe" : "bash"
    property string detectedBashCmd: FileUtils.getFilePath(StandardPaths.findExecutable(bashName));
    onDetectedBashCmdChanged: {
        console.debug('detectedBashCmd: ', detectedBashCmd)
    }

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    function getCygwinPath(cygpath, path) {

        console.debug('getCygwinPath: ', path)
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.outputChanged.connect((outputStringt) => {
                outputText += outputStringt;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from cygpath: \n' + outputText);
                try {
                    console.debug('accepting...: ' + outputText);
                    accept({launcher: launcher, outputText: outputText});
                    console.debug('accepted...: ' + outputText);
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            console.debug('cygpath: ', cygpath);
            launcher.execute(cygpath + " " + path);
            /*
            if(callback)
                callback(launcher)
                */
        })

        return promise;
    }

    function initExtraParams() {
        console.debug('resolving extra params..')

        if(Qt.platform.os === "windows") {
            var cygpath = FileUtils.getFilePath(StandardPaths.findExecutable("cygpath.exe"));
            console.debug('resolving extra params: ', cygpath)

            var dvRescueSh = FileUtils.getFilePath(FileUtils.find("dvrescue.sh"));
            var ffmpegSh = FileUtils.getFilePath(FileUtils.find("ffmpeg.sh"));
            var xmlSh = FileUtils.getFilePath(FileUtils.find("xml.sh"));
            var mediaInfoSh = FileUtils.getFilePath(FileUtils.find("mediainfo.sh"));

            var getDvRescueShPromise = getCygwinPath(cygpath, dvRescueSh).then((r) => { console.debug('getDvRescueShPromise: ', r.outputText); dvRescueSh = r.outputText });
            var getFfmpegShPromise = getCygwinPath(cygpath, ffmpegSh).then((r) => { console.debug('getFfmpegShPromise: ', r.outputText); ffmpegSh = r.outputText });
            var getXmlShPromise = getCygwinPath(cygpath, xmlSh).then((r) => { console.debug('getXmlShPromise: ', r.outputText); xmlSh = r.outputText });
            var getMediaInfoShPromise = getCygwinPath(cygpath, xmlSh).then((r) => { console.debug('getMediaInfoShPromise: ', r.outputText); mediaInfoSh = r.outputText });

            console.debug('Promise.all...');
            var waitAll = Promise.all([getDvRescueShPromise, getFfmpegShPromise, getXmlShPromise, getMediaInfoShPromise]).then(() => {
                console.debug('Promise.all finished...');
                extraParams = " -v -e mov -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
                               .replace("{xml}", xmlSh).replace("{ffmpeg}", ffmpegSh).replace("{dvrescue}", dvRescueSh).replace("{mediainfo}", mediaInfoSh)
            })
        } else {
            extraParams = " -v -e mov -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
                           .replace("{xml}", xmlStarletCmd).replace("{ffmpeg}", ffmpegCmd).replace("{dvrescue}", dvrescueCmd).replace("{mediainfo}", mediaInfoCmd)
        }

        return true;
    }

    property bool init: initExtraParams()
    property string extraParams: " -v "

    DvPackagerCtl {
        id: packagerCtl
        makeCmd: Qt.platform.os === "windows" ? function(cmd) {
            return detectedBashCmd + " " + cmd + extraParams;
        } : function(cmd) {
            return detectedBashCmd + " " + cmd + extraParams;
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

            if(Qt.platform.os === "windows") {
                packagerCtl.paths = [ FileUtils.getFileDir(dvrescueCmd), FileUtils.getFileDir(xmlStarletCmd),
                                      FileUtils.getFileDir(mediaInfoCmd), FileUtils.getFileDir(ffmpegCmd) ]
            }

            packagerCtl.exec(params, (launcher) => {
                debugView.logCommand(launcher)
                launcher.outputChanged.connect((outputString) => {
                    debugView.logResult(outputString);
                })
            }).then(() => {
                console.debug('executed....')
                busy.running = false;
            }).catch((error) => {
                busy.running = false;
            });
        }
    }
}
