import QtQuick
import "../LateNightTheme"

Item {
    id: root

    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#151515"
        visible: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0

        Image {
            anchors.fill: parent
            fillMode: Image.Tile
            source: LateNightTheme.optionalDeckControlsBackgroundTile
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderDark
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderDark
            width: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: LateNightTheme.deckPanelBorderLight
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderLight
            width: 1
        }
    }
}
