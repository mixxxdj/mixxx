import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Item {
    id: root

    required property real crossfaderWidth

    implicitHeight: crossfaderSlider.width

    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Skin.Slider {
        id: crossfaderSlider

        anchors.centerIn: parent
        height: root.crossfaderWidth
        group: "[Master]"
        key: "crossfader"
        barColor: Theme.crossfaderBarColor
        barStart: 0.5
        fg: "images/slider_handle_crossfader.svg"
        bg: "images/slider_crossfader.svg"

        transform: Rotation {
            origin.x: crossfaderSlider.width / 2
            origin.y: crossfaderSlider.height / 2
            angle: 90
        }

    }

    Item {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

}
