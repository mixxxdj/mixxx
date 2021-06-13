import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Knob {
    id: root

    property color color // required

    implicitWidth: background.width
    implicitHeight: implicitWidth
    arc: true
    arcRadius: width * 0.45
    arcOffsetY: width * 0.01
    arcColor: root.color
    arcWidth: 2
    angle: 116

    Image {
        id: shadow

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: width * 7 / 6
        fillMode: Image.PreserveAspectFit
        source: Theme.imgKnobMiniShadow
    }

    background: Image {
        id: background

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: width
        source: Theme.imgKnobMini
    }

    foreground: Item {
        id: inidicator

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: width

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width / 30
            height: 10
            y: height
            color: root.color
        }

    }

}
