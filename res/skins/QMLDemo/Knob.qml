import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Knob {
    id: root

    property color color // required

    implicitWidth: 56
    implicitHeight: implicitWidth
    arc: true
    arcRadius: 20
    arcColor: root.color
    arcWidth: 2
    angle: 117

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
