import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import DataModel 1.0
import QtAVPlayerUtils 1.0

ColumnLayout {
    id: root

    property alias segmentDataView: segmentDataView
    property alias currentIndex: segmentDataView.currentIndex
    property alias rowFilter: segmentDataView.rowFilter
    property alias hoveredItem: segmentDataView.hoveredItem
    property int framesCount: 0
    property var extraOpts: ({})
    onExtraOptsChanged: {
        if(reportPath != '')
            applyButton.clicked();
    }

    signal clicked(var index, var item);
    signal doubleClicked(var index, var item);
    signal populated()

    property string reportPath
    property string videoPath
    property string outputPath

    Rectangle {
        color: 'white'
        Layout.fillWidth: true
        Layout.minimumHeight: childrenRect.height

        Flow {
            id: segmentationOptionsLayout
            width: parent.width
            property bool needsApply: false;

            Label {
                text: "Segmenting Rules"
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                height: recordingStartMarkers.implicitHeight
            }

            CheckBox {
                id: recordingStartMarkers
                text: "Rec Start Markers"
                onCheckedChanged: {
                    segmentationOptionsLayout.needsApply = true
                }
            }
            CheckBox {
                id: breaksInRecordingTime
                text: "Rec Time Break"
                onCheckedChanged: {
                    segmentationOptionsLayout.needsApply = true
                }
            }
            CheckBox {
                id: breaksInTimecode
                text: "Timecode Break"
                onCheckedChanged: {
                    segmentationOptionsLayout.needsApply = true
                }
            }
            CheckBox {
                id: segmentFilesToPreserveAudioSampleRate
                text: "Audio Setting Change"
                onCheckedChanged: {
                    segmentationOptionsLayout.needsApply = true
                }
            }

            Label {
                text: "Aspect Ratio Change"
                verticalAlignment: Text.AlignVCenter
                height: recordingStartMarkers.implicitHeight
                font.bold: true
            }

            ComboBoxEx {
                id: aspectRatiosSelector
                sizeToContents: true
                model: [
                    "Yes, segment frames by aspect ratio changes",
                    "No and use most common aspect ratio",
                    "No and force segments to use 4/3",
                    "No and force segments to use 16/9"
                ]
                onCurrentIndexChanged: {
                    segmentationOptionsLayout.needsApply = true
                }
            }

            Button {
                text: "Reset"
                onClicked: {
                    if(recordingStartMarkers.checked) {
                        recordingStartMarkers.checked = false
                        segmentationOptionsLayout.needsApply = true
                    }

                    if(breaksInRecordingTime.checked) {
                        breaksInRecordingTime.checked = false
                        segmentationOptionsLayout.needsApply = true
                    }

                    if(breaksInTimecode.checked) {
                        breaksInTimecode.checked = false
                        segmentationOptionsLayout.needsApply = true
                    }

                    if(segmentFilesToPreserveAudioSampleRate.checked) {
                        segmentFilesToPreserveAudioSampleRate.checked = false
                        segmentationOptionsLayout.needsApply = true
                    }

                    if(aspectRatiosSelector.currentIndex !== 0) {
                        aspectRatiosSelector.currentIndex = 0
                        segmentationOptionsLayout.needsApply = true
                    }
                }
            }

            Button {
                id: applyButton
                enabled: segmentationOptionsLayout.needsApply
                text: "Apply"
                onClicked: {
                    segmentDataView.populateSegmentData(reportPath, videoPath, outputPath, extraOpts)
                    segmentationOptionsLayout.needsApply = false
                }
            }
        }
    }

    SegmentDataView {
        id: segmentDataView
        Layout.fillWidth: true
        Layout.fillHeight: true
        total: framesCount

        signal populated();

        onPopulated: {
            root.populated();
        }

        Component.onCompleted: {
            var e = {
                'Segment #' : '',
                'Frame #' : '',
                'Timestamp' : '',
                'Timecode' : '',
                'Timecode: Jump/Repeat' : Qt.point(0, 0),
                'Recording Time' : '',
                'Recording Time: Jump/Repeat' : Qt.point(0, 0),
                'Recording Marks' : Qt.point(0, 0),
                'Video/Audio' : ''
            }

            segmentDataView.model.appendRow(e);
            segmentDataView.model.clear();
        }

        SegmentDataParser {
            id: segmentDataParser
        }

        function packaging(reportPath, videoPath, outputDir, extraOpts, onOutputChanged) {

            if(outputDir === '') {
                outputDir = FileUtils.getFileDir(reportPath);
            }

            if(Qt.platform.os === "windows") {
                reportPath = "/cygdrive/" + reportPath.replace(":", "");
                videoPath = "/cygdrive/" + videoPath.replace(":", "");
                outputDir = "/cygdrive/" + outputDir.replace(":", "");
            }

            console.debug('packaging: reportPath = ', reportPath, 'videoPath = ', videoPath, 'outputDir = ', outputDir)

            var extraParams = " -v -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
            .replace("{xml}", packagerCtl.effectiveXmlStarletCmd)
            .replace("{ffmpeg}", packagerCtl.effectiveFfmpegCmd)
            .replace("{dvrescue}", packagerCtl.effectiveDvrescueCmd)
            .replace("{mediainfo}", packagerCtl.effectiveMediaInfoCmd)

            var opts = '-z ';
            if(recordingStartMarkers.checked)
                opts += '-s '
            if(breaksInRecordingTime.checked)
                opts += '-d ';
            if(breaksInTimecode.checked)
                opts += '-t ';
            if(segmentFilesToPreserveAudioSampleRate.checked)
                opts += '-3 ';

            if(aspectRatiosSelector.currentIndex === 0)
                opts += '-a n ';
            if(aspectRatiosSelector.currentIndex === 2)
                opts += '-a 4 ';
            if(aspectRatiosSelector.currentIndex === 3)
                opts += '-a 9 ';
            if(aspectRatiosSelector.currentIndex === 1)
                opts += '-a c ';

            if(extraOpts.type === 'mkv')
                opts += '-e mkv ';

            opts += '-o ' + outputDir + ' '

            var output = '';
            return packagerCtl.exec(opts + " -x " + reportPath + " " + videoPath, (launcher) => {
                                 debugView.logCommand(launcher)
                                 launcher.outputChanged.connect((outputString) => {
                                                                    if(onOutputChanged)
                                                                        onOutputChanged(outputString);
                                                                    output += outputString;
                                                                })

                                 launcher.errorChanged.connect((errorString) => {
                                                                    if(onOutputChanged)
                                                                        onOutputChanged(errorString);
                                                                    debugView.logResult(errorString);
                                                               })
                             }, extraParams).then(() => {
                                                      console.debug('executed packagerCtl....')
                                                      debugView.logResult(output);
                                                  });
        }

        function populateSegmentData(reportPath, videoPath, outputPath, extraOpts) {
            console.debug('populateSegmentData: reportPath = ', reportPath, 'outputPath = ', outputPath)

            segmentDataView.model.clear();

            if(Qt.platform.os === "windows") {
                reportPath = "/cygdrive/" + reportPath.replace(":", "");
                videoPath = "/cygdrive/" + videoPath.replace(":", "");
            }

            var extraParams = " -v -X {xml}".replace("{xml}", packagerCtl.effectiveXmlStarletCmd)

            var opts = ' ';
            if(recordingStartMarkers.checked)
                opts += '-s '
            if(breaksInRecordingTime.checked)
                opts += '-d ';
            if(breaksInTimecode.checked)
                opts += '-t ';
            if(segmentFilesToPreserveAudioSampleRate.checked)
                opts += '-3 ';

            if(aspectRatiosSelector.currentIndex === 0)
                opts += '-a n ';
            if(aspectRatiosSelector.currentIndex === 2)
                opts += '-a 4 ';
            if(aspectRatiosSelector.currentIndex === 3)
                opts += '-a 9 ';
            if(aspectRatiosSelector.currentIndex === 1)
                opts += '-a c ';

            if(extraOpts) {
                if(extraOpts.type === 'mkv')
                    opts += '-e mkv ';
            }

            if(outputPath) {
                if(Qt.platform.os === "windows") {
                    outputPath = "/cygdrive/" + outputPath.replace(":", "");
                }
                opts += '-o ' + outputPath + ' '
            }

            var output = '';
            packagerCtl.exec(opts + '-T' + ' ' + reportPath + ' ' + videoPath, (launcher) => {
                                 debugView.logCommand(launcher)
                                 launcher.outputChanged.connect((outputString) => {
                                                                    output += outputString;
                                                                })
                             }, extraParams).then(() => {
                                                      console.debug('executed....')
                                                      debugView.logResult(output);

                                                      var i = 0;
                                                      segmentDataParser.parse(output, (entry) => {
                                                                                  console.debug('entry: ', JSON.stringify(entry));

                                                                                  var videoAudio = [
                                                                                      entry.frameSize,
                                                                                      entry.frameRate,
                                                                                      entry.chromaSubsampling,
                                                                                      entry.aspectRatio,
                                                                                      entry.samplingRate,
                                                                                      entry.channelCount
                                                                                  ]

                                                                                  ++i
                                                                                  var e = {
                                                                                      'Segment #' : i,
                                                                                      'Frame #' : entry.startFrame,
                                                                                      'Timestamp' : entry.startPts,
                                                                                      'Timecode' : entry.timeCode,
                                                                                      'Timecode: Jump/Repeat' : Qt.point(entry.timeCodeJump, 0),
                                                                                      'Recording Time' : entry.recTimestamp,
                                                                                      'Recording Time: Jump/Repeat' : Qt.point(entry.recTimeJump, 0),
                                                                                      'Recording Marks' : Qt.point(entry.recStart, 0),
                                                                                      'Video/Audio' : videoAudio.join(' '),
                                                                                      'FileName' : entry.segmentFileName ? entry.segmentFileName : i
                                                                                  }

                                                                                  if(Qt.platform.os === "windows") {
                                                                                      var cygwinPath = e.FileName;
                                                                                      var splittedWinPath = cygwinPath.replace('/cygdrive/', '').split('/')
                                                                                      if(splittedWinPath.length !== 0) {
                                                                                        splittedWinPath[0] = splittedWinPath[0] + ':'
                                                                                        e.FileName = splittedWinPath.join('\\')
                                                                                      }
                                                                                  }

                                                                                  segmentDataView.model.appendRow(e);
                                                                              });
                                                      segmentDataView.populated();
                                                      busy.running = false;
                                                  }).catch((error) => {
                                                               debugView.logResult(error);
                                                               segmentDataView.populated();
                                                               busy.running = false;
                                                        });
        }
    }
}
