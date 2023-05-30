import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    property string statusGroup: root.knob.group
    required property string statusKey

    color: Theme.knobBackgroundColor
    width: 56
    height: 56
    radius: 5

    Skin.ControlKnob {
        id: knob

        anchors.centerIn: root
        width: 48
        height: 48
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.statusGroup
        key: root.statusKey
    }

    Rectangle {
        id: statusButton

        anchors.left: root.left
        anchors.bottom: root.bottom
        anchors.leftMargin: 4
        anchors.bottomMargin: 4
        width: 8
        height: width
        radius: width / 2
        border.width: 1
        border.color: Theme.buttonNormalColor
        color: statusControl.value ? knob.color : "transparent"

        TapHandler {
            onTapped: statusControl.value = !statusControl.value
        }
    }
}
