import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import MediaInfo 1.0

Instantiator {
    id: instantiator
    signal editRow(int index, string propertyName, var propertyValue);

    readonly property string filePathColumn: "File Path"
    readonly property string fileNameColumn: "File Name"
    readonly property string formatColumn: "Format"
    readonly property string fileSizeColumn: "File Size"
    readonly property string frameCountColumn: "Frame Count"
    readonly property string firstTimecodeColumn: "First Timecode"
    readonly property string lastTimecodeColumn: "Last Timecode"
    readonly property string firstRecordingTimeColumn: "First Recording Time"
    readonly property string lastRecordingTimeColumn: "Last Recording Time"
    readonly property string frameErrorColumn: "Frame Error %"
    readonly property string videoBlockErrorColumn: "Video Block Error %"
    readonly property string audioBlockErrorColumn: "Audio Block Error %"

    readonly property string progressRole: "Progress"
    readonly property string frameErrorTooltipRole: "Frame Error Tooltip"
    readonly property string videoBlockErrorTooltipRole: "Video Block Error Tooltip"
    readonly property string audioBlockErrorTooltipRole: "Audio Block Error Tooltip"
    readonly property string videoBlockErrorValueRole: "Video Block Error Value"
    readonly property string audioBlockErrorValueRole: "Audio Block Error Value"

    onModelChanged: {
        console.debug('model: ', model)
    }
    delegate: MediaInfo {
        reportPath: {
            console.debug('reportPath: ', index, filesModel.count)
            return (index >= 0 && index < filesModel.count) ? filesModel.get(index).reportPath : ''
        }
        videoPath: {
            console.debug('videoPath: ', index, filesModel.count)
            return (index >= 0 && index < filesModel.count) ? filesModel.get(index).videoPath : ''
        }

        function editRow(index, propertyName, propertyValue) {
            // console.debug('key: ', propertyName, 'value: ', JSON.stringify(propertyValue))
            if(index >= 0 && index < filesModel.count)
            {
                instantiator.editRow(index, propertyName, propertyValue)
            }
        }

        onFormatChanged: {
            editRow(index, formatColumn, format)
        }
        onFileSizeChanged: {
            editRow(index, fileSizeColumn, fileSize)
        }
        onFrameCountChanged: {
            editRow(index, frameCountColumn, frameCount)
        }
        onFirstTimeCodeChanged: {
            editRow(index, firstTimecodeColumn, firstTimeCode)
        }
        onLastTimeCodeChanged: {
        }
        onFirstRecordingTimeChanged: {
            editRow(index, firstRecordingTimeColumn, firstRecordingTime)
        }
        onLastRecordingTimeChanged: {
        }
        onParsingChanged: {
            editRow(index, lastTimecodeColumn, lastTimeCode)
            editRow(index, lastRecordingTimeColumn, lastRecordingTime)
            editRow(index, progressRole, parsing === false ? 1 : 0)
        }
        onBytesProcessedChanged: {
            editRow(index, progressRole, bytesProcessed / reportFileSize)
        }
        onFrameErrorChanged: {
            editRow(index, frameErrorColumn, frameError);
            editRow(index, frameErrorTooltipRole, "Sta count: " + staCount + ", Frames count: " + frameCount)
        }
        onVideoBlockErrorChanged: {
            editRow(index, videoBlockErrorColumn, videoBlockError);
            editRow(index, videoBlockErrorTooltipRole, "Sta sum: " + staSum + ", Sum of video blocks: " + totalVideoBlocks)
            editRow(index, videoBlockErrorValueRole, { x : videoBlockErrorValue.x, y : videoBlockErrorValue.y })
        }
        onAudioBlockErrorChanged: {
            editRow(index, audioBlockErrorColumn, audioBlockError);
            editRow(index, audioBlockErrorTooltipRole, "Aud sum: " + audSum + ", Sum of audio blocks: " + totalAudioBlocks)
            editRow(index, audioBlockErrorValueRole, { x : audioBlockErrorValue.x, y : audioBlockErrorValue.y })
        }

        Component.onCompleted: {
            resolve();
        }
    }
}
