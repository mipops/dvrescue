import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Rectangle {
    id: root
    signal sorting(int sortOrder);
    height: childrenRect.height
    property alias text: label.text
    property alias textFont: label.font
    property alias filterText: filterField.text
    property alias filterFont: filterField.font
    property bool canSort: true
    property alias state: upDownIndicator.state
    property bool canShowIndicator: true
    property bool canFilter: true

    readonly property int desiredWidth: metrics.width
    property int minimumWidth: 20

    clip: true
    color: "#333333"

    TextMetrics {
        id: metrics
        font: label.font
        text: label.text
    }

    Column {
        id: column
        spacing: 0

        TextFieldEx {
            id: filterField
            readOnly: !canFilter
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            placeholderText: canFilter ? "filter string" : ""
            placeholderTextColor: Qt.darker(label.color)
            color: '#aaaaaa'
            bottomPadding: 3
            topPadding: 0
        }

        Label {
            id: label
            color: '#aaaaaa'
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignHCenter

            property int initialSortOrder: Qt.DescendingOrder

            TapHandler { id: tap; onTapped: upDownIndicator.nextState(); enabled: canSort }

            Label {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 10
                id: upDownIndicator
                color: parent.color
                text: "^"
                property bool indicatorVisibility: false;
                visible: canShowIndicator && indicatorVisibility

                function nextState() {
                    if (state == "") {
                        state = (label.initialSortOrder == Qt.DescendingOrder ? "down" : "up")
                        root.sorting(Qt.DescendingOrder)
                    } else if (state == "up") {
                        state = "down"
                        root.sorting(Qt.DescendingOrder)
                    } else {
                        state = "up"
                        root.sorting(Qt.AscendingOrder)
                    }
                }

                states: [
                    State {
                        name: "up"
                        PropertyChanges { target: upDownIndicator; indicatorVisibility: true; rotation: 0 }
                    },
                    State {
                        name: "down"
                        PropertyChanges { target: upDownIndicator; indicatorVisibility: true; rotation: 180 }
                    }
                ]

            }
        }
    }

}
