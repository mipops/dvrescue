import QtQuick 2.0

Item {
    function parseDevicesList(value) {

        var devices = [];
        var splitted = value.split('\n');
        console.debug('splitted: ', splitted.length)

        if(splitted[0].indexOf('Devices:') !== -1)
        {
            splitted.shift();
            for(var i = 0; i < splitted.length; ++i)
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
