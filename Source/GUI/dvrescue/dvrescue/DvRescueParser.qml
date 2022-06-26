import QtQuick 2.0

Item {
    function parseDevicesList(value) {

        var devices = [];
        var splitted = value.split('\n');
        console.debug('splitted: ', splitted.length)

        for(var i = 0; i < splitted.length; ++i)
        {
            console.debug('parsing device entry: ', splitted[i]);

            var entry = splitted[i];
            var indexOfOpeningBracket = entry.indexOf('(', 0);
            if(indexOfOpeningBracket === -1)
                continue;

            var deviceName = entry.substr(0, indexOfOpeningBracket - 1).trim();

            var indexOfClosingBracket = entry.indexOf(')', indexOfOpeningBracket + 1);
            if(indexOfClosingBracket === -1)
                continue;

            var deviceType = entry.substr(indexOfOpeningBracket + 1, indexOfClosingBracket - indexOfOpeningBracket - 1);
            var deviceIndex = i

            console.debug('deviceIndex: ', deviceIndex, 'deviceName: ', deviceName, 'deviceType: ', deviceType);

            devices.push({
                            index: deviceIndex,
                            name: deviceName,
                            type: deviceType
                         })
        }

        return devices;
    }
}
