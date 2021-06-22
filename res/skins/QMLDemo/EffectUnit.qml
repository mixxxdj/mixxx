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

    Item {
        id: effectContainer

        anchors.margins: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: effectUnitControlsFrame.left
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

            PropertyChanges {
                target: superKnob
                visible: true
            }

            PropertyChanges {
                target: dryWetKnob
                visible: true
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
        id: effectUnitControlsFrame

        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: effectUnitControls.width
        color: Theme.knobBackgroundColor
        radius: 5

        Column {
            id: effectUnitControls

            anchors.top: parent.top
            anchors.right: parent.right
            padding: 5
            spacing: 10

            Item {
                width: 40
                height: width

                Skin.Button {
                    id: expandButton

                    anchors.fill: parent
                    activeColor: Theme.effectUnitColor
                    text: "▼"
                    checkable: true
                }

            }

            Skin.ControlKnob {
                id: superKnob

                height: 40
                width: height
                arcStart: Knob.ArcStart.Minimum
                group: "[EffectRack1_EffectUnit" + unitNumber + "]"
                key: "super1"
                color: Theme.effectUnitColor
                visible: false

                Skin.FadeBehavior on visible {
                    fadeTarget: superKnob
                }

            }

            Skin.ControlKnob {
                id: dryWetKnob

                height: 40
                width: height
                arcStart: Knob.ArcStart.Minimum
                group: "[EffectRack1_EffectUnit" + unitNumber + "]"
                key: "mix"
                color: Theme.effectUnitColor
                visible: false

                Skin.FadeBehavior on visible {
                    fadeTarget: dryWetKnob
                }

            }

            add: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 150
                }

                NumberAnimation {
                    property: "scale"
                    from: 0
                    to: 1
                    duration: 150
                }

            }

        }

    }

}
