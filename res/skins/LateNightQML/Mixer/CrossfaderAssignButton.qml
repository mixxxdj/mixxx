import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property string leftStyle: "default"
    property string rightStyle: "default"
    readonly property int state: Math.max(0, Math.min(2, Math.round(orientationControl.value)))
    readonly property string stateLabel: ["left", "mid", "right"][state]
    readonly property string stateStyle: {
        if (state === 0) {
            return root.leftStyle;
        } else if (state === 2) {
            return root.rightStyle;
        }
        return "warning";
    }

    implicitHeight: 15
    implicitWidth: 33

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: LateNightTheme.lateNightAsset("buttons", "btn__xfader_deck_" + root.stateLabel + "_" + root.stateStyle + ".svg")
    }
    MouseArea {
        anchors.fill: parent

        onClicked: orientationControl.value = (Math.round(orientationControl.value) + 1) % 3
    }
    Mixxx.ControlProxy {
        id: orientationControl

        group: root.group
        key: "orientation"
    }
}
