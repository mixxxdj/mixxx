import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Knob {
    id: root

    property color color // required

    width: 56
    height: width
    arc: true
    arcRadius: width * 0.35
    arcOffsetY: -width * 0.035
    arcColor: root.color
    arcWidth: 2
    angle: 117

    background: Image {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fillMode: Image.PreserveAspectFit
        mipmap: true
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
