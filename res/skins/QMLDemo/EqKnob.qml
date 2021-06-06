import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    property string statusGroup: root.knob.group // required
    property string statusKey // required

    color: "#282828"
    width: 56
    height: 56
    radius: 5

    Skin.ResettableControlKnob {
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
        anchors.leftMargin: 6
        anchors.bottomMargin: 2
        width: 8
        height: width
        radius: width / 2
        border.width: 1
        border.color: Theme.buttonNormalColor
        color: statusControl.value ? root.color : "transparent"

        MouseArea {
            anchors.fill: parent
            onClicked: statusControl.value = !statusControl.value
        }

    }

}
