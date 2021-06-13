import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Item {
    id: root

    property real crossfaderWidth // required

    implicitHeight: crossfader.height

    Skin.SectionBackground {
        anchors.fill: microphoneRow
    }

    Row {
        id: microphoneRow

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: crossfader.left
        padding: 5
        spacing: 20

        Skin.MicrophoneUnit {
            group: "[Microphone]"
        }

        Skin.MicrophoneUnit {
            group: "[Microphone2]"
        }

        Skin.MicrophoneUnit {
            group: "[Microphone3]"
        }

        Skin.MicrophoneUnit {
            group: "[Microphone4]"
        }

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

    Row {
        id: auxiliaryRow

        anchors.left: crossfader.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        layoutDirection: Qt.RightToLeft
        padding: 5
        spacing: 20

        Skin.AuxiliaryUnit {
            group: "[Auxiliary4]"
        }

        Skin.AuxiliaryUnit {
            group: "[Auxiliary3]"
        }

        Skin.AuxiliaryUnit {
            group: "[Auxiliary2]"
        }

        Skin.AuxiliaryUnit {
            group: "[Auxiliary1]"
        }

    }

}
