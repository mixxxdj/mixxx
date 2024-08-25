import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property alias knob: knob
    required property string group

    color: Theme.knobBackgroundColor
    width: 56
    height: 56
    radius: 5

    Skin.ControlKnob {
        id: knob

        group: root.group
        key: "super1"

        anchors.horizontalCenter: root.horizontalCenter
        anchors.top: root.top
        width: 40
        height: 40
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: "enabled"
    }

    Rectangle {
        id: statusButton

        anchors.left: root.left
        anchors.bottom: root.bottom
        anchors.leftMargin: 4
        anchors.bottomMargin: 4
        width: 8
        height: width
        radius: width / 2
        border.width: 1
        border.color: Theme.buttonNormalColor
        color: statusControl.value ? knob.color : "transparent"

        TapHandler {
            onTapped: statusControl.value = !statusControl.value
        }
    }

    Mixxx.ControlProxy {
        id: fxSelect

        group: root.group
        key: "loaded_chain_preset"
    }

    Skin.ComboBox {
        id: effectSelector
        anchors.left: statusButton.right
        anchors.leftMargin: 2
        anchors.right: root.right
        anchors.top: knob.bottom
        anchors.margins: 1
        spacing: 2
        indicator.width: 0
        popupWidth: 150
        clip: true

        opacity: statusControl.value ? 1 : 0.5
        textRole: "display"
        font.pixelSize: 10
        model: Mixxx.EffectsManager.quickChainPresetModel
        currentIndex: fxSelect.value == -1 ? 0 : fxSelect.value
        onActivated: (index) => {
            fxSelect.value = index
        }
    }
}
