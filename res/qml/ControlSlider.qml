import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12

Skin.Slider {
    property alias group: control.group
    property alias key: control.key

    value: control.parameter
    onMoved: control.parameter = value

    Mixxx.ControlProxy {
        id: control
    }

    TapHandler {
        onDoubleTapped: control.reset()
    }

}
