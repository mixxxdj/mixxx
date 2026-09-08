pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitHeight: 15
    implicitWidth: 33

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: {
            switch (Math.round(orientationControl.value)) {
            case 0:
                return LateNightTheme.assetAuxXfaderLeft;
            case 2:
                return LateNightTheme.assetAuxXfaderRight;
            default:
                return LateNightTheme.assetAuxXfaderMain;
            }
        }
    }
    Row {
        anchors.fill: parent

        Repeater {
            model: ["orientation_left", "orientation_center", "orientation_right"]

            Item {
                id: assignButton

                required property int index
                required property string modelData

                height: parent.height
                width: 11

                TapHandler {
                    onPressedChanged: assignControl.value = pressed ? 1 : 0
                }
                Mixxx.ControlProxy {
                    id: assignControl

                    group: root.group
                    key: assignButton.modelData
                }
            }
        }
    }
    Mixxx.ControlProxy {
        id: orientationControl

        group: root.group
        key: "orientation"
    }
}
