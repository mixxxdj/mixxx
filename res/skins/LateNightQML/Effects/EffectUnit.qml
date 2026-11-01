import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import "../../../qml" as Skin
import "../../../qml/Effects" as SharedEffects
import "../Controls" as LateNightControls
import "../LateNightTheme"

LateNightControls.Panel {
    id: root

    readonly property int collapsedChainWidth: collapsedSlotsWidth + flowWidth * 2
    readonly property int collapsedSlotsWidth: Math.max(0, Math.min(570, slots.width - (LateNightTheme.isPaleMoon ? 1 : 0) - flowWidth * 2 - mixerSeparatorWidth))
    readonly property color controllerColor: unitNumber < 3 ? LateNightTheme.effectsControllerColor12 : LateNightTheme.effectsControllerColor34
    readonly property bool expanded: expandedControl.value > 0
    readonly property int flowWidth: LateNightTheme.isPaleMoon ? 12 : 3
    readonly property string group: unit.group
    readonly property int headerWidth: expanded ? masterWidth : (LateNightTheme.isClassic ? 57 : 52)
    readonly property int masterWidth: expanded ? (LateNightTheme.isClassic ? 63 : 60) : (showSuper.value > 0 ? (LateNightTheme.isClassic ? 159 : 160) : (LateNightTheme.isClassic ? 122 : 123))
    readonly property int mixerSeparatorWidth: expanded ? 0 : (LateNightTheme.isClassic ? 3 : 5)
    readonly property Mixxx.EffectUnitProxy unit: Mixxx.EffectsManager.getEffectUnit(unitNumber)
    readonly property color unitColor: unitNumber < 3 ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitColor34
    required property int unitNumber

    borderVisible: LateNightTheme.isClassic
    bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
    color: LateNightTheme.effectsPanelColor
    implicitHeight: expanded ? (LateNightTheme.isPaleMoon ? 157 : Math.max(showSuper.value > 0 ? 150 : 120, slot1.implicitHeight + slot2.implicitHeight + slot3.implicitHeight + 10)) : (LateNightTheme.isClassic ? 42 : 36)
    leftBorderColor: LateNightTheme.mixerPanelBorderLeft
    radius: LateNightTheme.isClassic ? 2 : 1
    rightBorderColor: LateNightTheme.mixerPanelBorderRight
    topBorderColor: LateNightTheme.mixerPanelBorderTop

    LateNightControls.Panel {
        id: slots

        borderVisible: LateNightTheme.isPaleMoon
        color: LateNightTheme.effectsPanelColor
        height: parent.height - (LateNightTheme.isClassic ? 2 : 0)
        rightBorderColor: "transparent"
        width: parent.width - (root.expanded ? root.masterWidth + (LateNightTheme.isClassic ? 2 : 0) : root.masterWidth + root.headerWidth + (LateNightTheme.isClassic ? 2 : 0))
        x: LateNightTheme.isClassic ? 1 : 0
        y: x

        Image {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 154
            source: LateNightTheme.assetFxFlowVertical
            sourceSize: Qt.size(90, 154)
            visible: root.expanded && LateNightTheme.isPaleMoon
            width: 90
        }
        EffectSlot {
            id: slot1

            effectNumber: 1
            expanded: root.expanded
            height: root.expanded ? parent.height : (LateNightTheme.isClassic ? 36 : 33)
            parent: root.expanded ? slot1Frame : slots
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: root.expanded ? parent.width : Math.round(root.collapsedSlotsWidth / 3)
            x: root.expanded ? 0 : parent.width - root.collapsedChainWidth - root.mixerSeparatorWidth
            y: root.expanded ? 0 : (LateNightTheme.isClassic ? 2 : 1)
        }
        EffectSlot {
            id: slot2

            effectNumber: 2
            expanded: root.expanded
            height: root.expanded ? parent.height : (LateNightTheme.isClassic ? 36 : 33)
            parent: root.expanded ? slot2Frame : slots
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: root.expanded ? slot1.width : Math.round((root.collapsedSlotsWidth - slot1.width) / 2)
            x: root.expanded ? 0 : slot1.x + slot1.width + root.flowWidth
            y: root.expanded ? 0 : (LateNightTheme.isClassic ? 2 : 1)
        }
        EffectSlot {
            id: slot3

            effectNumber: 3
            expanded: root.expanded
            height: root.expanded ? parent.height : (LateNightTheme.isClassic ? 36 : 33)
            parent: root.expanded ? slot3Frame : slots
            unitColor: root.unitColor
            unitGroup: root.group
            unitNumber: root.unitNumber
            width: root.expanded ? slot1.width : root.collapsedSlotsWidth - slot1.width - slot2.width
            x: root.expanded ? 0 : slot2.x + slot2.width + root.flowWidth
            y: root.expanded ? 0 : (LateNightTheme.isClassic ? 2 : 1)
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: LateNightTheme.isClassic ? 30 : 34
            source: LateNightTheme.assetFxFlowHorizontal
            sourceSize: Qt.size(LateNightTheme.isClassic ? 1 : 12, height)
            visible: !root.expanded
            width: LateNightTheme.isClassic ? 1 : 12
            x: slot1.x + slot1.width + (LateNightTheme.isClassic ? 1 : 0)
            y: Math.round((parent.height - height) / 2)
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: LateNightTheme.isClassic ? 30 : 34
            source: LateNightTheme.assetFxFlowHorizontal
            sourceSize: Qt.size(LateNightTheme.isClassic ? 1 : 12, height)
            visible: !root.expanded
            width: LateNightTheme.isClassic ? 1 : 12
            x: slot2.x + slot2.width + (LateNightTheme.isClassic ? 1 : 0)
            y: Math.round((parent.height - height) / 2)
        }
        ColumnLayout {
            anchors.bottomMargin: 2
            anchors.fill: parent
            anchors.leftMargin: LateNightTheme.isClassic ? 3 : 2
            anchors.topMargin: LateNightTheme.isClassic ? 2 : 1
            spacing: 0
            visible: root.expanded

            Item {
                id: slot1Frame

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: slot1.implicitHeight
                Layout.preferredHeight: 50
            }
            SlotSeparator {
            }
            Item {
                id: slot2Frame

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: slot2.implicitHeight
                Layout.preferredHeight: 50
            }
            SlotSeparator {
            }
            Item {
                id: slot3Frame

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: slot3.implicitHeight
                Layout.preferredHeight: 50
            }
        }
    }
    Item {
        id: masterControls

        height: root.expanded ? root.height - 35 - (LateNightTheme.isClassic ? 2 : 0) : root.height - (LateNightTheme.isClassic ? 2 : 0)
        width: root.masterWidth
        x: root.expanded ? root.width - width - (LateNightTheme.isClassic ? 1 : 0) : root.width - root.headerWidth - width - (LateNightTheme.isClassic ? 1 : 0)
        y: (root.expanded ? 35 : 0) + (LateNightTheme.isClassic ? 1 : 0)

        ColumnLayout {
            id: masterLayout

            height: parent.height - (LateNightTheme.isClassic ? 0 : 5)
            spacing: 0
            visible: root.expanded
            width: 58
            x: LateNightTheme.isClassic ? 3 : 0
            y: LateNightTheme.isClassic ? 0 : 2

            Item {
                id: buttonsFrame

                Layout.fillWidth: true
                Layout.preferredHeight: 26
            }
            MasterSpring {
            }
            Item {
                id: presetFrame

                Layout.fillWidth: true
                Layout.preferredHeight: 22
            }
            Item {
                Layout.preferredHeight: 1
            }
            MasterSpring {
                Layout.maximumHeight: 15
            }
            Item {
                id: mixFrame

                Layout.fillWidth: true
                Layout.preferredHeight: 30
            }
            MasterSpring {
            }
            Item {
                id: superFrame

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                visible: showSuper.value > 0
            }
            MasterSpring {
            }
        }
        LateNightControls.Panel {
            anchors.fill: parent
            borderVisible: LateNightTheme.isPaleMoon
            color: LateNightTheme.effectsPanelColor
            leftBorderColor: "transparent"
            rightBorderColor: root.expanded ? LateNightTheme.mixerPanelBorderRight : "transparent"
            topBorderColor: root.expanded ? "transparent" : LateNightTheme.mixerPanelBorderTop
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
            pressedSource: LateNightTheme.assetFxMixModePressedButton
            width: 32
            x: root.expanded ? masterLayout.x : presetButton.x + presetButton.width + (LateNightTheme.isClassic ? 0 : 2)
            y: root.expanded ? masterLayout.y + buttonsFrame.y : (LateNightTheme.isClassic ? 7 : 4)
        }
        Image {
            anchors.centerIn: mixMode
            height: 24
            source: mixMode.active ? LateNightTheme.assetFxMixModeDryWetSumButton : LateNightTheme.assetFxMixModeDryWetButton
            width: 28
        }
        EffectControlButton {
            activeBackgroundSource: LateNightTheme.assetFxSlotButtonActiveBackground
            activeColor: LateNightTheme.mixerPflActiveFillColor
            activeSource: LateNightTheme.assetMixerPflActiveIcon
            backgroundSource: LateNightTheme.assetFxSlotButtonBackground
            group: root.group
            height: 26
            key: "group_[Headphone]_enable"
            normalColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            normalSource: LateNightTheme.assetMixerPflIcon
            width: 26
            x: mixMode.x + mixMode.width
            y: mixMode.y
        }
        Item {
            id: presetButton

            height: 22
            width: 22
            x: root.expanded ? masterLayout.x + 18 : mixKnob.x + mixKnob.width + (LateNightTheme.isClassic ? 1 : 2)
            y: root.expanded ? masterLayout.y + presetFrame.y : (LateNightTheme.isClassic ? 9 : 6)

            Image {
                height: 18
                source: LateNightTheme.assetFxSettingsButton
                width: 18
                x: 1
                y: 2
            }
            TapHandler {
                onTapped: presetPopup.open()
            }
        }
        LateNightControls.Knob {
            id: mixKnob

            backgroundSource: LateNightTheme.assetSmallKnobBackground
            displayArc: true
            displayArcColor: "#a00000"
            displayArcRadius: LateNightTheme.mixerArcRadiusCompact
            displayArcStart: LateNightControls.Knob.ArcStart.Minimum
            group: root.group
            height: 30
            indicatorColor: "red"
            indicatorKind: "small"
            key: "mix"
            width: 35
            x: root.expanded ? masterLayout.x + 11 : (LateNightTheme.isClassic ? (showSuper.value > 0 ? 41 : 4) : (showSuper.value > 0 ? 39 : 2))
            y: root.expanded ? masterLayout.y + mixFrame.y : (LateNightTheme.isClassic ? 5 : 2)
        }
        LateNightControls.Knob {
            backgroundSource: LateNightTheme.assetSmallKnobBackground
            displayArc: true
            displayArcColor: root.controllerColor
            displayArcRadius: LateNightTheme.mixerArcRadiusCompact
            displayArcStart: LateNightControls.Knob.ArcStart.Minimum
            group: root.group
            height: 30
            indicatorColor: root.unitNumber < 3 ? "green" : "blue"
            indicatorKind: "small"
            key: "super1"
            visible: showSuper.value > 0
            width: visible ? 35 : 0
            x: root.expanded ? masterLayout.x + 11 : (LateNightTheme.isClassic ? 4 : 2)
            y: root.expanded ? masterLayout.y + superFrame.y : (LateNightTheme.isClassic ? 5 : 2)
        }
    }
    Image {
        height: 30
        source: LateNightTheme.assetFxFlowHorizontal
        sourceSize.height: 30
        sourceSize.width: 1
        visible: !root.expanded && LateNightTheme.isClassic
        width: 1
        x: slots.x + slots.width - 2
        y: (root.height - height) / 2
    }
    Rectangle {
        color: LateNightTheme.deckPanelBorderDark
        height: root.height - 2
        visible: !root.expanded && LateNightTheme.isPaleMoon
        width: 1
        x: slots.width - 2
        y: 1
    }
    Rectangle {
        color: LateNightTheme.deckPanelBorderLight
        height: root.height - 2
        visible: !root.expanded && LateNightTheme.isPaleMoon
        width: 1
        x: slots.width - 1
        y: 1
    }
    Item {
        id: header

        height: root.expanded ? 35 : root.height - (LateNightTheme.isClassic ? 2 : 0)
        width: root.headerWidth
        x: root.width - width - (LateNightTheme.isClassic ? 1 : 0)
        y: LateNightTheme.isClassic ? 1 : 0
        z: 1002

        LateNightControls.Panel {
            anchors.fill: parent
            bottomBorderColor: LateNightTheme.isClassic ? "transparent" : root.expanded ? "transparent" : "#020202"
            color: LateNightTheme.effectsHeaderColor
            leftBorderColor: "transparent"
            radius: 1
            rightBorderColor: LateNightTheme.isClassic ? (controllerActive.value > 0 ? "#d09300" : "transparent") : "#111111"
            topBorderColor: LateNightTheme.isClassic ? (controllerActive.value > 0 ? "#d09300" : "transparent") : LateNightTheme.effectsHeaderBorderTopColor
        }
        Rectangle {
            color: root.controllerColor
            height: parent.height - (LateNightTheme.isClassic ? 4 : root.expanded ? 2 : 3)
            visible: controllerActive.value > 0
            width: 2
            x: LateNightTheme.isClassic ? 2 : 0
            y: LateNightTheme.isClassic ? 2 : 1
        }
        Text {
            color: controllerActive.value > 0 ? root.controllerColor : LateNightTheme.effectsHeaderInactiveTextColor
            font.family: "Open Sans"
            font.pixelSize: 16
            font.weight: LateNightTheme.isClassic ? Font.Bold : Font.Medium
            height: parent.height - (LateNightTheme.isClassic ? 0 : root.expanded ? 2 : 3)
            horizontalAlignment: Text.AlignLeft
            renderType: Text.NativeRendering
            text: "FX\u200a" + root.unitNumber
            verticalAlignment: Text.AlignVCenter
            width: LateNightTheme.isClassic ? 31 : 29
            x: LateNightTheme.isClassic ? (root.expanded ? 9 : 7) : (root.expanded ? 8 : 5)
            y: LateNightTheme.isClassic ? 0 : 1
        }
        Image {
            fillMode: Image.PreserveAspectFit
            height: 18
            source: root.expanded ? LateNightTheme.assetFxCollapseButton : LateNightTheme.assetFxExpandButton
            width: 16
            x: parent.width - width - (LateNightTheme.isClassic ? 3 : 1)
            y: Math.floor((parent.height - height - (LateNightTheme.isPaleMoon ? 1 : 0)) / 2)
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

    component MasterSpring: Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 1
        Layout.preferredHeight: 1
    }
    component SlotSeparator: Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 2

        Rectangle {
            color: LateNightTheme.deckPanelBorderDark
            height: 1
            width: Math.max(0, parent.width - 185)
        }
        Rectangle {
            color: LateNightTheme.deckPanelBorderLight
            height: 1
            width: Math.max(0, parent.width - 185)
            y: 1
        }
    }
}
