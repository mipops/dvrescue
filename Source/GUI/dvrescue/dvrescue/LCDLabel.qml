import QtQuick 2.0

Text {
    FontLoader {
        id: fontloader
        source: "fonts/DS-DIGI.TTF"
    }

    font.family: fontloader.name
}
