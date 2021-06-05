import "." as Skin
import Mixxx 0.1 as Mixxx

Skin.Knob {
    id: root

    property string group // required
    property string key // required

    onTurned: control.parameter = value

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
        onValueChanged: root.value = control.parameter
    }

}
