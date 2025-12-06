import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Skin.Fader {
    id: root

    required property string group
    required property string key

    value: control.parameter
    onMoved: control.parameter = value

    Mixxx.ControlProxy {
        id: control
        group: root.group
        key: root.key
    }

    TapHandler {
        onDoubleTapped: control.reset()
    }
}
