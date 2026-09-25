import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    required property string group
    property alias knob: knob

    color: Theme.knobBackgroundColor
    height: 42
    opacity: statusControl.value ? 1 : 0.4
    radius: 5
    width: 42

    Skin.ControlKnob {
        id: knob

        anchors.horizontalCenter: root.horizontalCenter
        anchors.top: root.top
        group: root.group
        height: 36
        key: "super1"
        width: 36
    }
    TapHandler {
        acceptedButtons: Qt.LeftButton

        onDoubleTapped: {
            statusControl.value = !statusControl.value;
        }
    }
    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: "enabled"
    }
}
