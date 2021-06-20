import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import "Theme"

Item {
    id: root

    property var slot: Mixxx.EffectsManager.getEffectSlot(1, unitNumber, effectNumber)
    property int unitNumber // required
    property int effectNumber // required
    property bool expanded: false
    readonly property string group: slot.group

    height: 50

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
        // TODO: Add a way to retrieve effect names here
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
        arcStart: 0
        width: 40
        group: root.group
        key: "meta"
        color: Theme.effectColor
    }

    Row {
        id: parameterContainer

        visible: root.expanded
        anchors.leftMargin: 10
        anchors.left: effectMetaKnob.right
        anchors.top: effectMetaKnob.top
        spacing: 5

        Repeater {
            model: 16

            Skin.ControlMiniKnob {
                id: parameterKnob

                property int parameterNumber: index + 1

                width: 40
                height: width
                arcStart: 0
                group: root.group
                key: "parameter" + parameterNumber
                color: Theme.effectColor
                visible: parameterLoadedControl.loaded

                Mixxx.ControlProxy {
                    id: parameterLoadedControl

                    property bool loaded: value != 0

                    group: root.group
                    key: "parameter" + parameterNumber + "_loaded"
                }

            }

        }

        Repeater {
            model: 16

            Item {
                id: buttonParameter

                property int parameterNumber: index + 1

                width: 40
                height: width
                visible: buttonParameterLoadedControl.loaded

                Mixxx.ControlProxy {
                    id: buttonParameterLoadedControl

                    property bool loaded: value != 0

                    group: root.group
                    key: "button_parameter" + parameterNumber + "_loaded"
                }

                Skin.ControlButton {
                    id: buttonParameterButton

                    height: 26
                    width: parent.width
                    anchors.centerIn: parent
                    group: root.group
                    key: "button_parameter" + parameterNumber
                    activeColor: Theme.effectColor
                }

            }

        }

        Skin.FadeBehavior on visible {
            fadeTarget: parameterContainer
        }

    }

}
