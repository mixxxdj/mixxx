import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    readonly property bool active: muteProxy.value > 0
    required property string group

    Rectangle {
        anchors.fill: parent
        color: root.active ? LateNightTheme.mixerAccentOrange : LateNightTheme.mixerStemMuteInactiveColor
    }
    Image {
        anchors.fill: parent
        source: root.active ? LateNightTheme.assetMixerEqKillButtonActiveBackground : LateNightTheme.assetMixerEqKillButtonBackground
    }
    Image {
        anchors.fill: parent
        source: LateNightTheme.assetMixerStemMuteIcon
        visible: !root.active
    }
    TapHandler {
        onTapped: muteProxy.value = muteProxy.value > 0 ? 0 : 1
    }
    Mixxx.ControlProxy {
        id: muteProxy

        group: root.group
        key: "mute"
    }
}
