import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group
    required property url iconSource
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
    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        height: 15
        source: root.iconSource
        visible: killControl.value <= 0
        width: 15
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
