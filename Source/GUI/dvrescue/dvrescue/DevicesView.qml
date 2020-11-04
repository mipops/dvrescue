import QtQuick 2.0
import QtQuick.Controls 2.12

ListView {
    delegate: Column {
        Text {
            text: index + " " + name + " " + type
        }

        Row {
            Button {
                text: 'rew'
            }
            Button {
                text: 'play'
            }
            Button {
                text: 'fwd'
            }
        }
    }
}
