pragma ComponentBehavior: Bound

import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls

Menu {
    id: root

    property color backgroundColor: "#151517"
    property color borderColor: "#333333"
    property url checkedSource
    property color disabledTextColor: "#494949"
    property string fontFamily: "Open Sans"
    property int fontPixelSize: 14
    property color hoverColor: "#2c454f"
    property color hoverTextColor: "#ffffff"
    property color separatorBottomColor: "#333333"
    property color separatorTopColor: "#000000"
    required property Mixxx.EffectSlotProxy slot1
    required property Mixxx.EffectSlotProxy slot2
    required property Mixxx.EffectSlotProxy slot3
    property url submenuArrowSource
    property color textColor: "#c2b3a5"
    required property Mixxx.EffectUnitProxy unit

    bottomPadding: 5
    leftPadding: 5
    rightPadding: 5
    topPadding: 5
    width: 160

    background: Rectangle {
        border.color: root.borderColor
        border.width: 1
        color: root.backgroundColor
        radius: 1
    }
    delegate: Skin.EffectPresetMenuItem {
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        submenuArrowSource: root.submenuArrowSource
        textColor: root.textColor
    }

    Instantiator {
        model: Mixxx.EffectsManager.standardChainPresetModel

        delegate: Skin.EffectPresetMenuItem {
            required property int index
            required property string name
            required property string presetDisplay
            required property string tooltip

            ToolTip.delay: 500
            ToolTip.text: tooltip
            ToolTip.visible: hovered && tooltip.length > 0
            checkedSource: root.checkedSource
            disabledTextColor: root.disabledTextColor
            fontFamily: root.fontFamily
            fontPixelSize: root.fontPixelSize
            hoverColor: root.hoverColor
            hoverTextColor: root.hoverTextColor
            submenuArrowSource: root.submenuArrowSource
            text: (root.unit.presetName === name ? "✓ " : "") + presetDisplay
            textColor: root.textColor

            onTriggered: root.unit.loadPreset(index)
        }

        onObjectAdded: (index, object) => root.insertItem(index, object)
        onObjectRemoved: (index, object) => root.removeItem(object)
    }
    Skin.EffectPresetMenuSeparator {
        bottomColor: root.separatorBottomColor
        topColor: root.separatorTopColor
    }
    Skin.EffectPresetMenuItem {
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        enabled: root.unit.canUpdatePreset
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        submenuArrowSource: root.submenuArrowSource
        text: qsTr("Update Preset")
        textColor: root.textColor
        visible: enabled

        onTriggered: root.unit.updatePreset()
    }
    Skin.EffectPresetMenuItem {
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        enabled: root.unit.canRenamePreset
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        submenuArrowSource: root.submenuArrowSource
        text: qsTr("Rename Preset")
        textColor: root.textColor
        visible: enabled

        onTriggered: root.unit.renamePreset()
    }
    Skin.EffectPresetMenuItem {
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        submenuArrowSource: root.submenuArrowSource
        text: qsTr("Save As New Preset...")
        textColor: root.textColor

        onTriggered: root.unit.savePresetAs()
    }
    Skin.EffectPresetMenuSeparator {
        bottomColor: root.separatorBottomColor
        topColor: root.separatorTopColor
    }
    Skin.EffectSlotPresetMenu {
        backgroundColor: root.backgroundColor
        borderColor: root.borderColor
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        separatorBottomColor: root.separatorBottomColor
        separatorTopColor: root.separatorTopColor
        slot: root.slot1
        submenuArrowSource: root.submenuArrowSource
        textColor: root.textColor
    }
    Skin.EffectSlotPresetMenu {
        backgroundColor: root.backgroundColor
        borderColor: root.borderColor
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        separatorBottomColor: root.separatorBottomColor
        separatorTopColor: root.separatorTopColor
        slot: root.slot2
        submenuArrowSource: root.submenuArrowSource
        textColor: root.textColor
    }
    Skin.EffectSlotPresetMenu {
        backgroundColor: root.backgroundColor
        borderColor: root.borderColor
        checkedSource: root.checkedSource
        disabledTextColor: root.disabledTextColor
        fontFamily: root.fontFamily
        fontPixelSize: root.fontPixelSize
        hoverColor: root.hoverColor
        hoverTextColor: root.hoverTextColor
        separatorBottomColor: root.separatorBottomColor
        separatorTopColor: root.separatorTopColor
        slot: root.slot3
        submenuArrowSource: root.submenuArrowSource
        textColor: root.textColor
    }
}
