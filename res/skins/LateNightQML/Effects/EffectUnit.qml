import Mixxx 1.0 as Mixxx
import QtQuick
import "../../../qml" as Skin
import "../../../qml/Effects" as SharedEffects
import "../Controls" as LateNightControls
import "../LateNightTheme"
import "../Mixer" as LateNightMixer

Item {
    id: root

    readonly property real collapsedChainWidth: collapsedSlotWidth * 3 + flowWidth * 2
    readonly property real collapsedSlotWidth: Math.min(182, (slots.width - flowWidth * 2) / 3)
    readonly property color controllerColor: unitNumber < 3 ? LateNightTheme.effectsControllerColor12 : LateNightTheme.effectsControllerColor34
    readonly property bool expanded: expandedControl.value > 0
    readonly property int flowWidth: LateNightTheme.isPaleMoon ? 12 : 3
    readonly property string group: unit.group
    readonly property int headerWidth: expanded ? 60 : 48
    readonly property int masterWidth: expanded ? 60 : (showSuper.value > 0 ? 160 : 123)
    readonly property Mixxx.EffectUnitProxy unit: Mixxx.EffectsManager.getEffectUnit(unitNumber)
    readonly property color unitColor: unitNumber < 3 ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitColor34
    required property int unitNumber

    implicitHeight: expanded ? 154 : 38

    Rectangle {
        anchors.fill: parent
        border.color: LateNightTheme.deckPanelBorderDark
        border.width: 1
        color: LateNightTheme.effectsPanelColor
        radius: 1
    }
    Item {
        id: slots

        height: parent.height
        width: parent.width - (root.expanded ? root.masterWidth : root.masterWidth + root.headerWidth)
        x: 0
        y: 0

        Image {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            fillMode: Image.Stretch
            source: LateNightTheme.assetFxFlowVertical
            visible: root.expanded && LateNightTheme.isPaleMoon
            width: 90
        }
        EffectSlot {
            id: slot1

            effectNumber: 1
            expanded: root.expanded
            height: root.expanded ? 51 : 34
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: root.expanded ? parent.width : root.collapsedSlotWidth
            x: root.expanded ? 0 : parent.width - root.collapsedChainWidth
            y: root.expanded ? 0 : 2
        }
        EffectSlot {
            id: slot2

            effectNumber: 2
            expanded: root.expanded
            height: root.expanded ? 51 : 34
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: slot1.width
            x: root.expanded ? 0 : slot1.x + slot1.width + root.flowWidth
            y: root.expanded ? 52 : 2
        }
        EffectSlot {
            id: slot3

            effectNumber: 3
            expanded: root.expanded
            height: root.expanded ? 50 : 34
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: slot1.width
            x: root.expanded ? 0 : slot2.x + slot2.width + root.flowWidth
            y: root.expanded ? 104 : 2
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: 34
            source: LateNightTheme.assetFxFlowHorizontal
            visible: !root.expanded
            width: root.flowWidth
            x: slot1.x + slot1.width
            y: 2
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: 34
            source: LateNightTheme.assetFxFlowHorizontal
            visible: !root.expanded
            width: root.flowWidth
            x: slot2.x + slot2.width
            y: 2
        }
        Item {
            height: 2
            visible: root.expanded
            width: Math.max(0, parent.width - 185)
            y: 51

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                color: LateNightTheme.deckPanelBorderDark
                height: 1
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                color: LateNightTheme.deckPanelBorderLight
                height: 1
            }
        }
        Item {
            height: 2
            visible: root.expanded
            width: Math.max(0, parent.width - 185)
            y: 103

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                color: LateNightTheme.deckPanelBorderDark
                height: 1
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                color: LateNightTheme.deckPanelBorderLight
                height: 1
            }
        }
    }
    Item {
        id: masterControls

        height: root.expanded ? root.height - 35 : root.height
        width: root.masterWidth
        x: root.expanded ? root.width - width : root.width - root.headerWidth - width
        y: root.expanded ? 35 : 0

        Rectangle {
            anchors.fill: parent
            border.color: LateNightTheme.deckPanelBorderDark
            border.width: 1
            color: LateNightTheme.effectsPanelColor
        }
        EffectControlButton {
            id: mixMode

            activeColor: LateNightTheme.effectsMasterButtonInactiveColor
            activeSource: LateNightTheme.assetFxMixModeButton
            group: root.group
            height: 26
            key: "mix_mode"
            normalColor: LateNightTheme.effectsMasterButtonInactiveColor
            normalSource: LateNightTheme.assetFxMixModeButton
            width: 32
            x: root.expanded ? 1 : presetButton.x + presetButton.width + 2
            y: root.expanded ? 2 : 6
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: mixMode.height
            source: mixMode.active ? LateNightTheme.assetFxMixModeDryWetSumButton : LateNightTheme.assetFxMixModeDryWetButton
            width: mixMode.width
            x: mixMode.x
            y: mixMode.y
        }
        LateNightMixer.PflButton {
            group: root.group
            height: 26
            key: "group_[Headphone]_enable"
            width: 26
            x: mixMode.x + mixMode.width
            y: mixMode.y
        }
        Item {
            id: presetButton

            height: 22
            width: 22
            x: root.expanded ? 19 : mixKnob.x + mixKnob.width + 2
            y: root.expanded ? (showSuper.value > 0 ? 30 : 43) : 8

            Image {
                anchors.fill: parent
                source: LateNightTheme.assetFxSettingsButton
            }
            TapHandler {
                onTapped: presetPopup.open()
            }
        }
        LateNightControls.Knob {
            id: mixKnob

            backgroundSource: LateNightTheme.assetSmallKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.mixerAccentRed
            displayArcRadius: 11.5
            displayArcStart: LateNightControls.Knob.ArcStart.Minimum
            group: root.group
            height: 30
            indicatorColor: "red"
            indicatorKind: "small"
            key: "mix"
            width: 35
            x: root.expanded ? 12 : (showSuper.value > 0 ? 39 : 2)
            y: root.expanded ? (showSuper.value > 0 ? 54 : 76) : 4
        }
        LateNightControls.Knob {
            backgroundSource: LateNightTheme.assetSmallKnobBackground
            displayArc: true
            displayArcColor: root.unitColor
            displayArcRadius: 11.5
            displayArcStart: LateNightControls.Knob.ArcStart.Minimum
            group: root.group
            height: 30
            indicatorColor: root.unitNumber < 3 ? "green" : "blue"
            indicatorKind: "small"
            key: "super1"
            visible: showSuper.value > 0
            width: visible ? 35 : 0
            x: root.expanded ? 12 : 2
            y: root.expanded ? 86 : 4
        }
    }
    Rectangle {
        anchors.fill: parent
        border.color: LateNightTheme.deckPanelBorderDark
        border.width: 1
        color: "transparent"
        radius: 1
    }
    Item {
        id: header

        height: root.expanded ? 35 : root.height
        width: root.headerWidth
        x: root.width - width
        y: 0

        Rectangle {
            anchors.fill: parent
            border.color: LateNightTheme.deckPanelBorderDark
            border.width: 1
            color: LateNightTheme.effectsHeaderColor
            radius: 1
        }
        Rectangle {
            color: root.controllerColor
            height: parent.height - 4
            visible: controllerActive.value > 0
            width: 2
            x: 1
            y: 2
        }
        Text {
            color: controllerActive.value > 0 ? root.controllerColor : "#686666"
            font.family: "Open Sans"
            font.pixelSize: 16
            font.weight: Font.Medium
            height: parent.height
            horizontalAlignment: Text.AlignLeft
            text: "FX\u200a" + root.unitNumber
            verticalAlignment: Text.AlignVCenter
            width: 31
            x: root.expanded ? 8 : 5
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: 18
            source: root.expanded ? LateNightTheme.assetFxCollapseButton : LateNightTheme.assetFxExpandButton
            width: 16
            x: parent.width - width - 1
            y: Math.round((parent.height - height) / 2)
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: root.controllerColor
            height: 1
            visible: controllerActive.value > 0
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            color: root.controllerColor
            visible: controllerActive.value > 0
            width: 1
        }
        TapHandler {
            onTapped: expandedControl.value = root.expanded ? 0 : 1
        }
    }
    SharedEffects.EffectChainPresetPopup {
        id: presetPopup

        backgroundColor: LateNightTheme.isClassic ? "#0f0f0f" : "#151517"
        borderColor: LateNightTheme.isClassic ? "#888888" : "#333333"
        checkedSource: LateNightTheme.lateNightAsset("buttons", LateNightTheme.isClassic ? "btn__lib_checkmark_orange.svg" : "btn__lib_checkmark_blue.svg")
        disabledTextColor: "#c2b3a5"
        hoverColor: LateNightTheme.isClassic ? "#5e4507" : "#2c454f"
        separatorBottomColor: LateNightTheme.isClassic ? "#222222" : "#333333"
        slot1: slot1.slot
        slot2: slot2.slot
        slot3: slot3.slot
        submenuArrowSource: LateNightTheme.lateNightAsset("style", LateNightTheme.isClassic ? "menu_arrow_yellow.svg" : "menu_arrow_ivory.svg")
        unit: root.unit
        x: masterControls.x + presetButton.x - 3
        y: masterControls.y + presetButton.y + presetButton.height + 2
    }
    Mixxx.ControlProxy {
        id: expandedControl

        group: root.group
        key: "show_parameters"
    }
    Mixxx.ControlProxy {
        id: controllerActive

        group: root.group
        key: "controller_input_active"
    }
    Mixxx.ControlProxy {
        id: showSuper

        group: "[Skin]"
        key: "show_superknobs"
    }
}
