import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    required property string group

    color: Theme.knobBackgroundColor
    width: 48
    height: 48
    radius: 5
    opacity: statusControl.value ? 0.4 : 1

    TapHandler {
        onDoubleTapped: {
            statusControl.value = !statusControl.value
        }
    }

    Skin.ControlKnob {
        id: knob

        group: root.group
        key: "super1"

        anchors.horizontalCenter: root.horizontalCenter
        anchors.top: root.top
        width: 42
        height: 42
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: "enabled"
    }
}
