import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12

Skin.ControlKnob {
    id: root

    property string resetGroup: group
    property string resetKey: key + "_set_default"

    Mixxx.ControlProxy {
        id: resetControl

        group: root.resetGroup
        key: root.resetKey
    }

    TapHandler {
        grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.ApprovesTakeOverByAnything
        onDoubleTapped: {
            resetControl.value = 1;
            resetControl.value = 0;
        }
    }

}
