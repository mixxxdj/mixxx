import QtQuick
import "../LateNightTheme"

Rectangle {
    border.color: LateNightTheme.rackFillerBorderColor
    border.width: 1
    color: LateNightTheme.effectsFillerColor
    radius: LateNightTheme.isClassic ? 2 : 1

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: LateNightTheme.rackFillerTopBorderColor
        height: 1
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        color: LateNightTheme.rackFillerLeftBorderColor
        width: 1
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: LateNightTheme.rackFillerBottomBorderColor
        height: 1
    }
}
