import '../Defines' as Defines

import QtQuick 2.15

Item {
    anchors.fill: parent

    Defines.Colors {
        id: colors
    }

    property int deckId: 0

    Rectangle {
        id: keyBackground
        width: 60
        height: 20
        color: deckInfo.isKeyLockOn ? colors.musicalKeyColors[deckInfo.keyIndex] : colors.musicalKeyColorsDark[deckInfo.keyIndex]
        anchors.right: parent.right
        anchors.top: parent.top
        Rectangle {
            id: keyBorder
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: keyBackground.width -2
            height: keyBackground.height -2
            color: "transparent"
            border.color: colors.defaultBackground
            border.width: 2
        }
    }

    Text {
        text: deckInfo.hasKey && (deckInfo.keyAdjustString != "-0") && (deckInfo.keyAdjustString != "+0") ? (settings.camelotKey ? utils.camelotConvert(deckInfo.keyString) : deckInfo.keyString) + deckInfo.keyAdjustString
        : deckInfo.hasKey ? (settings.camelotKey ? utils.camelotConvert(deckInfo.keyString) : deckInfo.keyString)
        : "No key"
        color: deckInfo.isKeyLockOn ? "black" : "white"
        font.pixelSize: 15
        font.family: "Pragmatica"
        anchors.fill: keyBackground
        anchors.rightMargin: 2
        anchors.topMargin: 1
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
