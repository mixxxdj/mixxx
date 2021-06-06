import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12

Skin.ControlSlider {
    property string resetGroup: group
    property string resetKey: key + "_set_default"

    Mixxx.ControlProxy {
        id: resetControl

        group: resetGroup
        key: resetKey
    }

    TapHandler {
        onDoubleTapped: {
            resetControl.value = 1;
            resetControl.value = 0;
        }
    }

}
