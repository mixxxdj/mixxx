import "." as Skin
import QtQuick 2.12
import "Theme"

Item {
    id: root

    required property string group

    Rectangle {
        id: gainKnobFrame

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: Theme.knobBackgroundColor
        height: width
        radius: 5

        Skin.ControlKnob {
            id: gainKnob

            anchors.centerIn: parent
            color: Theme.gainKnobColor
            group: root.group
            height: 36
            key: "pregain"
            width: 36
        }
    }
    Item {
        anchors.bottom: pflButton.top
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: gainKnobFrame.bottom
        anchors.topMargin: 5

        Skin.VuMeter {
            group: root.group
            height: parent.height - 22
            key: "vu_meter_left"
            width: 4
            x: 15
            y: (parent.height - height) / 2
        }
        Skin.VuMeter {
            group: root.group
            height: parent.height - 22
            key: "vu_meter_right"
            width: 4
            x: parent.width - width - 15
            y: (parent.height - height) / 2
        }
        Skin.ControlFader {
            id: volumeSlider

            anchors.fill: parent
            barColor: Theme.volumeSliderBarColor
            bg: Theme.imgVolumeSliderBackground
            group: root.group
            key: "volume"

            handleImage {
                width: parent.width - 4
            }
        }
    }
    Skin.ControlButton {
        id: pflButton

        activeColor: Theme.pflActiveButtonColor
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        group: root.group
        key: "pfl"
        text: "PFL"
        toggleable: true
    }
}
