import QtQuick 2.0

Item {
    property string modeChanged: "Mode changed:"
    property string speedChanged: "Speed changed:"
    property string status: "status:"

    function parseStatus(value) {
        var statusOutput = "2021-08-27 13:33:24.348 avfctl[99766:4607139] Device [0] DV-VCR (Sony HVR-M15AU) status: stopped\n"
        var statusObject = {};

        var indexOfStatus = value.indexOf(status);
        if(indexOfStatus !== -1)
        {
            var statusValue = value.substr(indexOfStatus + status.length).trim();
            statusObject.statusText = statusValue;
        }

        return statusObject;
    }

    function parseStateChanged(value) {
        var playOutput = "2020-11-06 05:40:25.509 avfctl[23782:7704522] Mode changed: 0 -> 1
2020-11-06 05:40:25.510 avfctl[23782:7704522] Speed changed: 0.000000 -> 1.000000";

        var state = {};

        var splitted = value.split('\n');
        console.debug('splitted: ', splitted.length)

        for(var i = 0; i < splitted.length; ++i)
        {
            var entry = splitted[i];
            var indexOfModeChanged = entry.indexOf(modeChanged)
            if(indexOfModeChanged !== -1)
            {
                var substr = entry.substr(indexOfModeChanged + modeChanged.length).trim()
                var fromTo = substr.split(' -> ');
                if(fromTo.length === 2)
                {
                    state.mode = { from: Number(fromTo[0]), to: Number(fromTo[1]) }
                }
            }

            var indexOfSpeedChanged = entry.indexOf(speedChanged)
            if(indexOfSpeedChanged !== -1)
            {
                var substr = entry.substr(indexOfSpeedChanged + speedChanged.length).trim()
                var fromTo = substr.split(' -> ');
                if(fromTo.length === 2)
                {
                    state.speed = { from: Number(fromTo[0]), to: Number(fromTo[1]) }
                }
            }
        }

        return state;
    }

    function parseDevicesList(value) {

        var devices = [];
        var splitted = value.split('\n');
        console.debug('splitted: ', splitted.length)

        var indexOfDevices = -1;
        for(var i = 0; i < splitted.length; ++i)
        {
            if(splitted[i].indexOf('Devices:') !== -1)
            {
                indexOfDevices = i;
                break;
            }
        }

        if(indexOfDevices !== -1)
        {
            ++indexOfDevices;
            for(var i = indexOfDevices; i < splitted.length; ++i)
            {
                var entry = splitted[i];
                if(entry === '')
                    continue;

                var indexOfOpeningBracket = entry.indexOf('[')
                indexOfOpeningBracket = entry.indexOf('[', indexOfOpeningBracket + 1);
                var indexOfClosingBracket = entry.indexOf(']', indexOfOpeningBracket + 1);

                var deviceIndex = entry.substr(indexOfOpeningBracket + 1, indexOfClosingBracket - indexOfOpeningBracket - 1);

                indexOfOpeningBracket = entry.indexOf('(', indexOfClosingBracket);
                var deviceName = entry.substr(indexOfClosingBracket + 1, indexOfOpeningBracket - indexOfClosingBracket - 1).trim();

                indexOfClosingBracket = entry.indexOf(')', indexOfOpeningBracket + 1);
                var deviceType = entry.substr(indexOfOpeningBracket + 1, indexOfClosingBracket - indexOfOpeningBracket - 1);

                console.debug('deviceIndex: ', deviceIndex, 'deviceName: ', deviceName, 'deviceType: ', deviceType);

                devices.push({
                                index: deviceIndex,
                                name: deviceName,
                                type: deviceType
                             })
            }
        }

        return devices;
    }
}
