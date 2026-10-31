import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import "Theme"

MixxxControls.Knob {
    id: root

    property url backgroundSource: Theme.imgKnob
    required property color color
    property url shadowSource: Theme.imgKnobShadow
    property bool showDefaultBackground: true
    property bool showDefaultForeground: true

    angle: 116
    arc: true
    arcColor: root.color
    arcOffsetY: width * 0.01
    arcRadius: width * 0.45
    arcWidth: 2
    implicitHeight: implicitWidth
    implicitWidth: background.width

    background: Image {
        id: background

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: width
        source: root.backgroundSource
        visible: root.showDefaultBackground
    }
    foreground: Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: width
        visible: root.showDefaultForeground

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            color: root.color
            height: root.width / 5
            width: 2
            y: height
        }
    }

    Image {
        id: shadow

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        fillMode: Image.PreserveAspectFit
        height: width * 7 / 6
        source: root.shadowSource
        visible: root.showDefaultBackground
    }
}
