import "." as Skin
import Mixxx 0.1 as Mixxx

Skin.Slider {
    property alias group: control.group
    property alias key: control.key

    value: control.parameter
    onMoved: control.parameter = value

    Mixxx.ControlProxy {
        id: control
    }

}
