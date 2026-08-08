import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    property string activeStyle: "default"
    required property string group
    required property string label
    required property int side
    readonly property string stateName: {
        if (Math.round(orientationControl.value) !== root.side) {
            return "off";
        }
        return root.activeStyle;
    }

    implicitHeight: 15
    implicitWidth: 11

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: LateNightTheme.lateNightAsset("buttons", "btn__xfader_deck_" + root.label + "_" + root.stateName + ".svg")
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
