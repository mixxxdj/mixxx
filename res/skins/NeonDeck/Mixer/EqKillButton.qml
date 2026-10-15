import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as Controls
import "../LateNightTheme"

Item {
    id: root

    required property string group
    /// Font Awesome icon name. A crossed circle reads as "cut this band".
    property string iconName: "ban"
    required property string key
    property alias value: killControl.value

    function toggle() {
        killControl.value = killControl.value > 0 ? 0 : 1;
    }

    implicitHeight: 18
    implicitWidth: 18

    Rectangle {
        anchors.fill: parent
        color: killControl.value > 0 ? LateNightTheme.mixerEqKillActiveColor : "#111113"
    }
    Image {
        anchors.fill: parent
        source: killControl.value > 0 ? LateNightTheme.assetMixerEqKillButtonActiveBackground : LateNightTheme.assetMixerEqKillButtonBackground
    }
    Controls.IconLabel {
        anchors.centerIn: parent
        iconColor: LateNightTheme.textColor
        iconName: root.iconName
        iconSize: 13
        visible: killControl.value <= 0
    }
    TapHandler {
        onTapped: killControl.value = killControl.value > 0 ? 0 : 1
    }
    Mixxx.ControlProxy {
        id: killControl

        group: root.group
        key: root.key
    }
}
