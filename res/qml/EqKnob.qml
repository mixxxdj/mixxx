import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    property string statusGroup: root.knob.group
    required property string statusKey

    color: Theme.knobBackgroundColor
    height: 42
    opacity: statusControl.value ? 0.4 : 1
    radius: 5
    width: 42

    TapHandler {
        onDoubleTapped: {
            statusControl.value = !statusControl.value;
        }
    }
    Skin.ControlKnob {
        id: knob

        anchors.centerIn: root
        height: 36
        width: 36
    }
    Mixxx.ControlProxy {
        id: statusControl

        group: root.statusGroup
        key: root.statusKey
    }
}
