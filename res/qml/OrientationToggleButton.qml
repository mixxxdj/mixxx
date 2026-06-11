import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    required property string key
    property alias orientation: orientationSlider.value
    property color color: "white"

    implicitWidth: 56
    implicitHeight: 26

    Skin.Fader {
        id: orientationSlider

        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        wheelEnabled: false
        live: false
        from: 0
        to: 2
        stepSize: 1
        value: control.value
        orientation: Qt.Horizontal
        snapMode: Fader.SnapOnRelease
        onMoved: {
            // The slider's `value` is not updated until after the move ended.
            const val = valueAt(visualPosition);
            if (val != control.value)
                control.value = val;
        }

        background: Rectangle {
            id: sliderBackground

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: 2
            color: root.color
        }

        handle: Rectangle {
            id: indicator

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 5
            x: orientationSlider.visualPosition * (sliderBackground.width - width)
            width: 3
            color: root.color

            Behavior on x {
                NumberAnimation {
                    duration: 150
                }
            }
        }
    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }
}
