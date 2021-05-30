import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.11
import "Theme"

Item {
    id: root

    required property string group

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

    Slider {
        id: volumeSlider

        anchors.top: gainKnob.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: pflButton.top
        group: root.group
        key: "volume"
        barColor: Theme.volumeSliderBarColor
        bg: "images/slider_volume.svg"
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
