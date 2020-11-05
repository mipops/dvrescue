import QtQuick 2.0
import Launcher 0.1

Item {
    property string avfctlCmd: "plink dave -batch /Users/test/Downloads/dvrescue-git/tools/avfctl/avfctl"

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    AvfCtlParser {
        id: parser
    }

    function queryDecks() {
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept(parser.parseDevicesList(outputText));
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -list_devices");
        })

        return promise;
    }
}
