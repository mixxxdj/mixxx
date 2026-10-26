import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Skin.Knob {
    id: root

    property alias controlProxy: control
    property alias group: control.group
    property alias key: control.key

    value: control.parameter

    onTurned: control.parameter = value

    Mixxx.ControlProxy {
        id: control
    }
    TapHandler {
        enabled: root.inputEnabled
        onDoubleTapped: control.reset()
    }
    TapHandler {
        enabled: root.inputEnabled
        acceptedButtons: Qt.RightButton

        onPressedChanged: {
            if (pressed) {
                control.reset();
            }
        }
    }
}
