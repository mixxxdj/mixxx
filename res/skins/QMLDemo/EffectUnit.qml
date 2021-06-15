import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "Theme"

Item {
    id: root

    property int unitNumber // required

    Skin.SectionBackground {
        anchors.fill: parent
    }

    RowLayout {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: effectSuperKnobFrame.left
        anchors.rightMargin: 5

        Repeater {
            model: 3

            Item {
                id: effect

                property var slot: Mixxx.EffectsManager.getEffectSlot(1, root.unitNumber, index + 1)
                readonly property int effectNumber: slot.number
                readonly property string group: slot.group

                height: 50
                Layout.fillWidth: true

                Skin.ControlButton {
                    id: effectEnableButton

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: 5
                    width: 40
                    group: effect.group
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
                        if (effect.slot.effectId != effectId)
                            effect.slot.effectId = effectId;

                    }

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

                        target: effect.slot
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
                    group: effect.group
                    key: "meta"
                    color: Theme.effectColor
                }

            }

        }

    }

    Rectangle {
        id: effectSuperKnobFrame

        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        color: Theme.knobBackgroundColor
        radius: 5

        Skin.ControlKnob {
            id: effectSuperKnob

            anchors.centerIn: parent
            width: 48
            height: 48
            arcStart: 0
            group: "[EffectRack1_EffectUnit" + unitNumber + "]"
            key: "super1"
            color: Theme.effectUnitColor
        }

    }

}
