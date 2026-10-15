import QtQuick
import "../LateNightTheme"

/// Solo button of a stem row: while it is on, the other stems of the deck stay
/// muted so the stem can be auditioned on its own.
Item {
    id: root

    required property bool active

    signal toggled

    Rectangle {
        anchors.fill: parent
        color: root.active ? LateNightTheme.mixerAccentCyan : LateNightTheme.mixerStemMuteInactiveColor
    }
    Image {
        anchors.fill: parent
        source: root.active ? LateNightTheme.assetMixerEqKillButtonActiveBackground : LateNightTheme.assetMixerEqKillButtonBackground
    }
    Text {
        anchors.centerIn: parent
        color: root.active ? "#000000" : LateNightTheme.toolbarButtonInactiveTextColor
        font.family: LateNightTheme.fontFamily
        font.pixelSize: 11
        font.weight: Font.Bold
        text: "S"
    }
    TapHandler {
        onTapped: root.toggled()
    }
}
