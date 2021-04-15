import QtQuick 2.0
import Qt.labs.platform 1.1
import Launcher 0.1
import FileUtils 1.0

Item {
    property string cmd

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    function makeReport(file, callback) {
        console.debug('making report: ', file);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
            });
            launcher.processFinished.connect(() => {
                console.debug('got from dvrescue: \n' + outputText);
                try {
                    accept();
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            var makeReportTemplate = "file.dv -x file.dv.dvrescue.xml -s file.dv.dvrescue.vtt -c file.dv.dvrescue.scc";
            var arguments = makeReportTemplate.split(" ");
            for(var i = 0; i < arguments.length; ++i) {
                arguments[i] = arguments[i].replace("file.dv", file);
            }

            launcher.execute(cmd, arguments);
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
