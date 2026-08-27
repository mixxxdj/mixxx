import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12

Item {
    id: root

    property color color: "white"
    required property string group
    required property string key
    property alias orientation: orientationSlider.value

    implicitHeight: 26
    implicitWidth: 56

    Skin.Fader {
        id: orientationSlider

        anchors.bottomMargin: 2
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 2
        from: 0
        live: false
        orientation: Qt.Horizontal
        snapMode: MixxxControls.Slider.SnapOnRelease
        stepSize: 1
        to: 2
        value: control.value
        wheelEnabled: false

        background: Rectangle {
            id: sliderBackground

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            color: root.color
            height: 2
        }
        handle: Rectangle {
            id: indicator

            anchors.bottom: parent.bottom
            anchors.margins: 5
            anchors.top: parent.top
            color: root.color
            width: 3
            x: orientationSlider.visualPosition * (sliderBackground.width - width)

            Behavior on x {
                NumberAnimation {
                    duration: 150
                }
            }
        }

        onMoved: function (value) {
            if (value != control.value)
                control.value = value;
        }
    }
    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }
}
