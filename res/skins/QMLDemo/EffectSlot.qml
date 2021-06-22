import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import "Theme"

Item {
    id: root

    property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(1, unitNumber, effectNumber)
    property int unitNumber // required
    property int effectNumber // required
    property bool expanded: false
    readonly property string group: slot.group
    property real maxSelectorWidth: 300

    height: 50

    Item {
        id: selector

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Math.min(root.width, root.maxSelectorWidth)

        Skin.ControlButton {
            id: effectEnableButton

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 5
            width: 40
            group: root.group
            key: "enabled"
            toggleable: true
            text: "ON"
            activeColor: Theme.effectColor
        }

        Skin.ComboBox {
            id: effectSelector

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: effectEnableButton.right
            anchors.right: effectMetaKnob.left
            anchors.margins: 5
            textRole: "display"
            model: Mixxx.EffectsManager.visibleEffectsModel
            onActivated: {
                const effectId = model.get(index).effectId;
                if (root.slot.effectId != effectId)
                    root.slot.effectId = effectId;

            }
            Component.onCompleted: root.slot.onEffectIdChanged()

            Connections {
                function onEffectIdChanged() {
                    const rowCount = effectSelector.model.rowCount();
                    // TODO: Consider using an additional QHash in the
                    // model and provide a more efficient lookup method
                    for (let i = 0; i < rowCount; i++) {
                        if (effectSelector.model.get(i).effectId === target.effectId) {
                            effectSelector.currentIndex = i;
                            break;
                        }
                    }
                }

                target: root.slot
            }

        }

        Skin.ControlMiniKnob {
            id: effectMetaKnob

            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 5
            arcStart: Knob.ArcStart.Minimum
            width: 40
            group: root.group
            key: "meta"
            color: Theme.effectColor
        }

    }

    ListView {
        id: parametersView

        visible: root.expanded
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.left: selector.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        spacing: 5
        model: root.slot.parametersModel
        orientation: ListView.Horizontal

        delegate: Item {
            id: parameter

            property int number: index + 1
            // TODO: Use null coalescing when we switch to Qt >= 5.15
            property string label: shortName ? shortName : name
            property string key: controlKey
            property bool isButton: controlHint > 0 && controlHint == 6
            property bool isKnob: controlHint > 0 && controlHint < 6

            width: 50
            height: 50

            EmbeddedText {
                anchors.fill: parent
                verticalAlignment: Text.AlignBottom
                text: parameter.label
                font.bold: false
            }

            Skin.ControlMiniKnob {
                id: parameterKnob

                width: 30
                height: 30
                anchors.centerIn: parent
                arcStart: 0
                group: root.group
                key: parameter.key
                color: Theme.effectColor
                visible: parameter.isKnob

                Mixxx.ControlProxy {
                    id: parameterLoadedControl

                    property bool loaded: value != 0

                    group: root.group
                    key: parameter.key + "_loaded"
                }

            }

            Skin.ControlButton {
                id: buttonParameterButton

                height: 22
                width: parent.width
                anchors.centerIn: parent
                group: root.group
                key: parameter.key
                activeColor: Theme.effectColor
                visible: parameter.isButton
                toggleable: true
                text: "ON"

                Mixxx.ControlProxy {
                    id: buttonParameterLoadedControl

                    property bool loaded: value != 0

                    group: root.group
                    key: parameter.key + "_loaded"
                }

            }

        }

        populate: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }

            NumberAnimation {
                property: "scale"
                from: 0
                to: 1
                duration: 200
            }

        }

        Skin.FadeBehavior on opacity {
            fadeTarget: parametersView
        }

    }

}
