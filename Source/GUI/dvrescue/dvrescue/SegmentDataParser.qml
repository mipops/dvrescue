import QtQuick 2.0

Item {
    function parse(segmentData, onEntryParsed) {

        var splitted = segmentData.split('\n');
        for(var i = 0; i < splitted.length; ++i) {
            if(splitted[i].length === 0)
                continue;

            var columns = splitted[i].split('|');

            var entry = {
                startPts: columns[0],
                endPts: columns[1],
                startFrame: columns[2],
                endFrame: columns[3],
                timeCode: columns[4],
                offsetStart: columns[5],
                recTimestamp: columns[6],
                frameSize: columns[7],
                frameRate: columns[8],
                chromaSubsampling: columns[9],
                aspectRatio: columns[10],
                samplingRate: columns[11],
                channelCount: columns[12],
                recStart: columns[13],
                recTimeJump: columns[14],
                timeCodeJump: columns[15],
                endsWithMissingAudioData: columns[16],
                offsetAtEndOfTheRange: columns[17],
                offsetOfLastFrameOfTheRange: columns[18],
                segmentFileName: columns.length > 20 ? columns[20] : ''
            }

            if(onEntryParsed)
                onEntryParsed(entry, columns)
        }
    }
}
