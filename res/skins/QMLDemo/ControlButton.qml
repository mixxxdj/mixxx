import "." as Skin
import Mixxx 0.1 as Mixxx

Skin.Button {
    id: root

    property string group // required
    property string key // required
    property bool toggleable: false

    highlight: control.value
    onPressed: {
        if (toggleable)
            control.value = !control.value;
        else
            control.value = 1;
    }
    onReleased: {
        if (!toggleable)
            control.value = 0;

    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

}
