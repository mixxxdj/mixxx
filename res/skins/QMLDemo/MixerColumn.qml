import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.11
import "Theme"

Item {
    id: root

    property string group // required

    Rectangle {
        id: gainKnobFrame

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: width
        color: Theme.knobBackgroundColor
        radius: 5

        Skin.ControlKnob {
            id: gainKnob

            anchors.centerIn: parent
            width: 48
            height: 48
            group: root.group
            key: "pregain"
            color: Theme.gainKnobColor
        }

    }

    Item {
        anchors.top: gainKnobFrame.bottom
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: pflButton.top

        Skin.VuMeter {
            x: 15
            y: (parent.height - height) / 2
            width: 4
            height: parent.height - 40
            group: root.group
            key: "VuMeterL"
        }

        Skin.VuMeter {
            x: parent.width - width - 15
            y: (parent.height - height) / 2
            width: 4
            height: parent.height - 40
            group: root.group
            key: "VuMeterR"
        }

        Skin.ControlSlider {
            id: volumeSlider

            anchors.fill: parent
            group: root.group
            key: "volume"
            barColor: Theme.volumeSliderBarColor
            bg: Theme.imgVolumeSliderBackground
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
        toggleable: true
    }

}
