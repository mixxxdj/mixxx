import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import "Effects" as Effects

Item {
    id: root

    implicitHeight: columns.height

    Row {
        id: columns

        width: root.width

        Column {
            width: parent.width / 2

            Effects.EffectUnit {
                unitNumber: 1
                width: parent.width
            }
            Effects.EffectUnit {
                unitNumber: 3
                visible: showFourUnitsControl.value > 0
                width: parent.width
            }
        }
        Column {
            width: parent.width / 2

            Effects.EffectUnit {
                unitNumber: 2
                width: parent.width
            }
            Effects.EffectUnit {
                unitNumber: 4
                visible: showFourUnitsControl.value > 0
                width: parent.width
            }
        }
    }
    Skin.SectionBackground {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.top: parent.top
        z: -1
    }
    Skin.SectionBackground {
        anchors.bottom: parent.bottom
        anchors.left: parent.horizontalCenter
        anchors.right: parent.right
        anchors.top: parent.top
        z: -1
    }
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
