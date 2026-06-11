import "." as Skin
import QtQuick 2.12
import "Theme"

Item {
    id: root

    required property real crossfaderWidth

    implicitHeight: crossfader.height

    Skin.SectionBackground {
        anchors.fill: microphoneRow
    }
    Row {
        id: microphoneRow

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: crossfader.left
        anchors.top: parent.top
        layoutDirection: Qt.RightToLeft
        padding: 5
        spacing: 10

        Skin.MicrophoneUnit {
            unitNumber: 1
        }
        Skin.MicrophoneUnit {
            unitNumber: 2
        }
        Skin.MicrophoneUnit {
            unitNumber: 3
        }
        Skin.MicrophoneUnit {
            unitNumber: 4
        }
        Skin.MicrophoneDuckingPanel {
        }
    }
    Skin.SectionBackground {
        id: crossfader

        anchors.centerIn: parent
        height: crossfaderSlider.height + 20
        width: root.crossfaderWidth

        Skin.ControlFader {
            id: crossfaderSlider

            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            barColor: Theme.crossfaderBarColor
            barStart: 0.5
            bg: Theme.imgCrossfaderBackground
            fg: Theme.imgCrossfaderHandle
            group: "[Master]"
            key: "crossfader"
            orientation: Qt.Horizontal
        }
    }
    Row {
        id: auxiliaryRow

        anchors.bottom: parent.bottom
        anchors.left: crossfader.right
        anchors.right: parent.right
        anchors.top: parent.top
        padding: 5
        spacing: 10

        Skin.AuxiliaryUnit {
            layoutDirection: Qt.RightToLeft
            unitNumber: 1
        }
        Skin.AuxiliaryUnit {
            layoutDirection: Qt.RightToLeft
            unitNumber: 2
        }
        Skin.AuxiliaryUnit {
            layoutDirection: Qt.RightToLeft
            unitNumber: 3
        }
        Skin.AuxiliaryUnit {
            layoutDirection: Qt.RightToLeft
            unitNumber: 4
        }
    }
}
