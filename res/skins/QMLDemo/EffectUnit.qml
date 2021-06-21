import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Item {
    id: root

    property int unitNumber // required

    implicitHeight: effectContainer.height

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Item {
        id: effectContainer

        anchors.margins: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: effectSuperKnobFrame.left
        height: 60

        EffectSlot {
            id: effect1

            anchors.top: parent.top
            anchors.left: parent.left
            width: parent.width / 3
            unitNumber: root.unitNumber
            effectNumber: 1
            expanded: false
        }

        EffectSlot {
            id: effect2

            anchors.top: parent.top
            anchors.left: effect1.right
            width: parent.width / 3
            unitNumber: root.unitNumber
            effectNumber: 2
            expanded: false
        }

        EffectSlot {
            id: effect3

            anchors.top: parent.top
            anchors.left: effect2.right
            width: parent.width / 3
            unitNumber: root.unitNumber
            effectNumber: 3
            expanded: false
        }

        states: State {
            when: expandButton.checked
            name: "expanded"

            AnchorChanges {
                target: effect1
                anchors.left: effectContainer.left
            }

            AnchorChanges {
                target: effect2
                anchors.left: effectContainer.left
                anchors.top: effect1.bottom
            }

            AnchorChanges {
                target: effect3
                anchors.left: effectContainer.left
                anchors.top: effect2.bottom
            }

            PropertyChanges {
                target: effect1
                width: parent.width
                expanded: true
            }

            PropertyChanges {
                target: effect2
                width: parent.width
                expanded: true
            }

            PropertyChanges {
                target: effect3
                width: parent.width
                expanded: true
            }

            PropertyChanges {
                target: effectContainer
                height: 160
            }

        }

        transitions: Transition {
            AnchorAnimation {
                targets: [effect1, effect2, effect3]
                duration: 150
            }

        }

    }

    Rectangle {
        id: effectSuperKnobFrame

        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        height: 50
        width: effectSuperKnob.width + expandButton.width + 15
        color: Theme.knobBackgroundColor
        radius: 5

        Skin.ControlKnob {
            id: effectSuperKnob

            anchors.margins: 5
            anchors.left: parent.left
            anchors.top: parent.top
            height: 48
            width: height
            arcStart: Knob.ArcStart.Minimum
            group: "[EffectRack1_EffectUnit" + unitNumber + "]"
            key: "super1"
            color: Theme.effectUnitColor
        }

        Skin.Button {
            id: expandButton

            anchors.margins: 5
            anchors.left: effectSuperKnob.right
            anchors.top: parent.top
            height: 40
            width: height
            activeColor: Theme.effectUnitColor
            text: "▼"
            checkable: true
        }

    }

}
