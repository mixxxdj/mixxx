import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.11
import "Theme"

Item {
    id: root

    property string group // required

    Knob {
        id: gainKnob

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: width
        group: root.group
        key: "pregain"
        color: Theme.gainKnobColor
    }

    Item {
        anchors.top: gainKnob.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: pflButton.top

        VuMeter {
            x: 15
            y: (parent.height - height) / 2
            width: 4
            height: parent.height - 40
            group: root.group
            key: "VuMeterL"
        }

        VuMeter {
            x: parent.width - width - 15
            y: (parent.height - height) / 2
            width: 4
            height: parent.height - 40
            group: root.group
            key: "VuMeterR"
        }

        Slider {
            id: volumeSlider

            anchors.fill: parent
            group: root.group
            key: "volume"
            barColor: Theme.volumeSliderBarColor
            bg: "images/slider_volume.svg"
        }

    }

    Skin.ControlButton {
        id: pflButton

        group: root.group
        key: "pfl"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "PFL"
        activeColor: Theme.pflActiveButtonColor
        checkable: true
    }

}
