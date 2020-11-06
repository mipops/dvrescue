import QtQuick 2.0
import QtQuick.Controls 2.12

ListView {
    id: listView
    signal playClicked(var index);
    signal stopClicked(var index);
    signal moveToStartClicked(var index);
    signal moveToEndClicked(var index);

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
        }
    }
}
