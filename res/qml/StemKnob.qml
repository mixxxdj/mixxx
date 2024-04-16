import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob

    required property string group
    required property string label
    required property color stemColor
    required property int index

    width: 56
    height: 56
    radius: 5
    color: stemColor

    Skin.ControlKnob {
        id: knob
        group: root.group
        key: `stem_${root.index}_volume`
        color: Theme.gainKnobColor
        anchors.topMargin: 5
        anchors.top: root.top
        anchors.horizontalCenter: root.horizontalCenter

        arcStart: 0

        width: 36
        height: 36
    }

    Text {
        anchors.bottom: root.bottom
        anchors.horizontalCenter: root.horizontalCenter
        text: label
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: `stem_${root.index}_mute`
    }

    Rectangle {
        id: statusButton

        anchors.left: root.left
        anchors.top: root.top
        anchors.leftMargin: 4
        anchors.topMargin: 4
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
