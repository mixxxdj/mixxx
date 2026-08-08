import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    readonly property real enabledValue: enabledControl.value
    required property string quickEffectGroup

    function toggle() {
        enabledControl.value = enabledControl.value > 0 ? 0 : 1;
    }

    implicitHeight: 18
    implicitWidth: 18

    Rectangle {
        anchors.fill: parent
        border.color: "#050505"
        border.width: 2
        color: enabledControl.value > 0 ? LateNightTheme.mixerQuickEffectActiveColor : "#111113"
    }
    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        height: 18
        source: LateNightTheme.assetMixerQuickEffectIcon
        visible: enabledControl.value <= 0
        width: 18
    }
    MouseArea {
        anchors.fill: parent

        onClicked: root.toggle()
    }
    Mixxx.ControlProxy {
        id: enabledControl

        group: root.quickEffectGroup
        key: "enabled"
    }
}
