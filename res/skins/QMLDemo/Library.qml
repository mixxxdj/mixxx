import "." as Skin
import Qt.labs.qmlmodels 1.0
import QtQuick 2.12
import QtQuick.Controls 1.4
import "Theme"

Item {
    Rectangle {
        color: Theme.deckBackgroundColor
        anchors.fill: parent

        Text {
            text: "Library Placeholder"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            color: Theme.deckTextColor
            anchors.centerIn: parent
        }

    }

}
