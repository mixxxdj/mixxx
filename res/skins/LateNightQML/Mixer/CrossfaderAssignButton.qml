pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property string leftStyle: "default"
    property string rightStyle: "default"
    readonly property int state: Math.max(0, Math.min(2, Math.round(orientationControl.value)))
    readonly property string stateLabel: ["left", "mid", "right"][state]
    readonly property string stateStyle: {
        if (state === 0) {
            return root.leftStyle;
        } else if (state === 2) {
            return root.rightStyle;
        }
        return "warning";
    }

    implicitHeight: 15
    implicitWidth: 33

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: LateNightTheme.lateNightAsset("buttons", "btn__xfader_deck_" + root.stateLabel + "_" + root.stateStyle + ".svg")
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

                MouseArea {
                    anchors.fill: parent

                    onCanceled: assignControl.value = 0
                    onPressed: assignControl.value = 1
                    onReleased: assignControl.value = 0
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
