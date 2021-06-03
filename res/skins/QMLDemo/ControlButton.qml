import "." as Skin
import Mixxx 0.1 as Mixxx

Skin.Button {
    id: root

    property string group // required
    property string key // required
    property bool toggle: false

    highlight: control.value
    onPressed: {
        if (!toggle)
            control.value = 1;
        else
            control.value = !control.value;
    }
    onReleased: {
        if (!toggle)
            control.value = 0;

    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

}
