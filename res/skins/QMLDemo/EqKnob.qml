import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Knob {
    id: root

    property color color // required
    property string statusGroup: root.group // required
    property string statusKey // required

    implicitWidth: 56
    implicitHeight: width
    arc: true
    arcRadius: 20
    arcColor: root.color
    arcWidth: 2
    angle: 117

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

    background: Image {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        source: "images/knob.svg"
    }

    foreground: Item {
        anchors.fill: parent

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width / 30
            height: root.width / 7
            y: parent.height / 4
            color: root.color
        }

    }

}
