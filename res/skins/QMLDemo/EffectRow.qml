import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    width: positioner.width
    height: positioner.height

    Row {
        id: positioner

        Skin.EffectUnit {
            id: effectUnit1

            width: root.width / 2
            unitNumber: 1
        }

        Skin.EffectUnit {
            id: effectUnit2

            width: root.width / 2
            unitNumber: 2
        }

    }

    Skin.SectionBackground {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.horizontalCenter
    }

    Skin.SectionBackground {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.horizontalCenter
        anchors.right: parent.right
    }

}
