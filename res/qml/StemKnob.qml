import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: volume

    required property string stemGroup
    required property string label
    required property color stemColor

    readonly property string fxGroup: `[QuickEffectRack1_${stemGroup}]`

    width: 56
    height: 56
    radius: 5
    color: stemColor
    opacity: statusControl.value ? 0.5 : 1

    Mixxx.ControlProxy {
        id: statusControl

        group: root.stemGroup
        key: "mute"
    }

    Mixxx.ControlProxy {
        id: fxControl

        group: root.fxGroup
        key: "enabled"
    }

    Rectangle {
        id: statusButton

        anchors.left: root.left
        anchors.top: root.top
        anchors.leftMargin: 4
        anchors.topMargin: 4
        width: 8
        height: width
        radius: width / 2
        border.width: 1
        border.color: Theme.buttonNormalColor
        color: statusControl.value ? volume.color : "transparent"

        TapHandler {
            onTapped: statusControl.value = !statusControl.value
        }
    }

    Text {
        id: stemLabel
        anchors.top: root.top
        anchors.right: root.right
        anchors.topMargin: 2
        anchors.rightMargin: 2
        elide: Text.ElideRight
        text: label
        font.pixelSize: 10
    }

    Skin.ControlKnob {
        id: volume
        group: root.stemGroup
        key: "volume"
        color: Theme.gainKnobColor
        anchors.leftMargin: 1
        anchors.top: statusButton.bottom
        anchors.left: root.left

        arcStart: 0

        width: 32
        height: 32
    }

    Rectangle {
        id: fxButton

        anchors.top: stemLabel.bottom
        anchors.right: root.right
        anchors.left: volume.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        width: 8
        height: width
        radius: width / 2
        border.width: 1
        border.color: Theme.buttonNormalColor
        color: fxControl.value ? effectSuperKnob.color : "transparent"

        TapHandler {
            onTapped: fxControl.value = !fxControl.value
        }
    }

    Skin.ControlMiniKnob {
        id: effectSuperKnob

        anchors.right: root.right
        anchors.left: volume.right
        anchors.bottom: effectSelector.top
        anchors.margins: 2
        arcStart: Knob.ArcStart.Minimum
        group: root.fxGroup
        key: "super1"
        color: Theme.effectColor
        opacity: fxControl.value ? 1 : 0.5
    }

    Mixxx.ControlProxy {
        id: fxSelect

        group: root.fxGroup
        key: "loaded_chain_preset"
    }

    Skin.ComboBox {
        id: effectSelector
        anchors.left: root.left
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.margins: 1
        spacing: 2
        indicator.width: 0
        popupWidth: 150
        clip: true

        opacity: fxControl.value ? 1 : 0.5
        textRole: "display"
        font.pixelSize: 10
        model: Mixxx.EffectsManager.quickChainPresetModel
        currentIndex: fxSelect.value == -1 ? 0 : fxSelect.value
        onActivated: (index) => {
            fxSelect.value = index
        }
    }
}
