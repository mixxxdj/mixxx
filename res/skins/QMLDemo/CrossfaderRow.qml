import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Item {
    id: root

    property real crossfaderWidth // required

    implicitHeight: crossfader.height

    Skin.EffectUnit {
        id: effectUnit1

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: crossfader.left
        anchors.bottom: parent.bottom
        unitNumber: 1
    }

    Skin.SectionBackground {
        id: crossfader

        anchors.centerIn: parent
        width: root.crossfaderWidth
        height: crossfaderSlider.height + 20

        Skin.ControlSlider {
            id: crossfaderSlider

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            orientation: Qt.Horizontal
            group: "[Master]"
            key: "crossfader"
            barColor: Theme.crossfaderBarColor
            barStart: 0.5
            fg: Theme.imgCrossfaderHandle
            bg: Theme.imgCrossfaderBackground
        }

    }

    Skin.EffectUnit {
        id: effectUnit2

        anchors.top: parent.top
        anchors.left: crossfader.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        unitNumber: 2
    }

}
