import QtQuick
import "../LateNightTheme"

Rectangle {
    id: root

    property bool borderVisible: true
    property color topBorderColor: LateNightTheme.deckPanelBorderLight
    property color leftBorderColor: LateNightTheme.deckPanelBorderLeft
    property color bottomBorderColor: LateNightTheme.deckPanelBorderDark
    property color rightBorderColor: LateNightTheme.deckPanelBorderRight

    color: LateNightTheme.backgroundColor
    radius: 1

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: root.topBorderColor
        visible: root.borderVisible
        z: 1000
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: root.leftBorderColor
        visible: root.borderVisible
        z: 1000
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: root.bottomBorderColor
        visible: root.borderVisible
        z: 1000
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: root.rightBorderColor
        visible: root.borderVisible
        z: 1000
    }
}
