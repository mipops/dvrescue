import QtQuick 2.0

Item {
    function parseDevicesList(value) {

        var devices = []
        var jsonDevices = JSON.parse(value);
        for(var i = 0; i < jsonDevices.length; ++i) {
            var jsonDevice = jsonDevices[i];

            var id = jsonDevice.id;
            var name = jsonDevice.name;
            var type = jsonDevice.type;

            console.debug('id: ', id, 'deviceName: ', name, 'deviceType: ', type);

            devices.push({
                            index: id,
                            name: name,
                            type: type
                         })
        }

        return devices;
    }
}
