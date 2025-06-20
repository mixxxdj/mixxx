import "." as Skin
import Mixxx 1.0 as Mixxx

Skin.Button {
    id: root

    required property string group
    required property string key
    property bool toggleable: false

    function toggle() {
        control.value = !control.value;
    }

    highlight: control.value
    onPressed: {
        if (toggleable)
            toggle();
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
