import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.0

ListView {
    id: listView
    signal playClicked(var index);
    signal stopClicked(var index);
    signal moveToStartClicked(var index);
    signal moveToEndClicked(var index);
    signal grabClicked(var index, var filePath);

    property var urlToPath: function(url) {
        return url;
    }

    FileDialog {
        id: specifyPathDialog
        selectExisting: false
        property var callback;

        onAccepted: {
            var fileUrl = specifyPathDialog.fileUrl;
            console.debug('selected file: ', fileUrl);
            if(callback)
                callback(fileUrl);
        }
    }

    delegate: Column {
        onWidthChanged: {
            if(listView.width < width)
                listView.width = width;
        }

        spacing: 5

        Text {
            text: index + " " + name + " " + type
        }

        Row {
            spacing: 5
            Button {
                text: 'rew'
                onClicked: moveToStartClicked(model.index);
            }
            Button {
                text: 'play'
                onClicked: playClicked(model.index);
            }
            Button {
                text: 'stop'
                onClicked: stopClicked(model.index);
            }
            Button {
                text: 'fwd'
                onClicked: moveToEndClicked(model.index);
            }
            Button {
                text: 'grab'
                enabled: grabPath.text.length !== 0
                onClicked: grabClicked(model.index, grabPath.text);
            }
            TextField {
                id: grabPath
                placeholderText: 'specify file path...'
            }
            ToolButton {
                onClicked: {
                    specifyPathDialog.callback = (fileUrl) => {
                        grabPath.text = urlToPath(fileUrl);
                    };

                    specifyPathDialog.open();
                }
            }
        }
    }
}
