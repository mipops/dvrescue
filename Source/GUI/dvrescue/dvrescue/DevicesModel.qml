import QtQuick 2.0

ListModel {
    function exists(device) {
        for(var j = 0; j < count; ++j) {
            var modelDevice = get(j);
            if(devicesEqual(device, modelDevice))
                return true;
        }

        return false;
    }

    function devicesEqual(device1, device2) {
        return device1.index === device2.index && device1.name === device2.name && device1.type === device2.type
    }

    function existsInDevices(devices, device) {
        for(var i = 0; i < devices.length; ++i) {
            if(devicesEqual(devices[i], device))
                return true;
        }

        return false;
    }

    function update(devices) {
        console.debug('got ', devices.length, 'devices')

        var newDevices = []
        for(var j = count - 1; j >= 0; --j) {
            var modelDevice = get(j);
            if(!existsInDevices(devices, modelDevice))
            {
                console.debug('removing ', modelDevice.index, modelDevice.name, modelDevice.type);
                remove(j, 1);
            }
        }

        for(var i = 0; i < devices.length; ++i) {
            var device = devices[i];
            if(!exists(device)) {
                newDevices.push(device);
            }
        }

        newDevices.forEach((device) => {
            console.debug('adding ', device.index, device.name, device.type);
            append({index: device.index, name: device.name, type: device.type})
        });
    }
}
