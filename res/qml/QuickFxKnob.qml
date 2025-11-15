import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    required property string group

    color: Theme.knobBackgroundColor
    width: 42
    height: 42
    radius: 5
    opacity: statusControl.value ? 1 : 0.4

    Skin.ControlKnob {
        id: knob

        group: root.group
        key: "super1"

        anchors.horizontalCenter: root.horizontalCenter
        anchors.top: root.top
        width: 36
        height: 36
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onDoubleTapped: {
            statusControl.value = !statusControl.value
        }
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: "enabled"
    }
}
