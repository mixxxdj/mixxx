pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    property int activeButtonParameterCount: 0
    property int activeKnobParameterCount: 0
    readonly property real buttonParameterWidth: 55 + parameterInitialGrowth
    required property int effectNumber
    property bool expanded: false
    readonly property color focusInactiveBorderColor: LateNightTheme.isClassic ? LateNightTheme.deckPanelBorderDark : "transparent"
    readonly property bool focused: showFocus.value > 0 && Math.round(focusedEffect.value) === effectNumber
    readonly property real knobParameterWidth: 40 + parameterInitialGrowth + (activeKnobParameterCount > 0 ? Math.max(0, parameterExtraWidth - parameterInitialGrowth * (activeButtonParameterCount + activeKnobParameterCount)) / activeKnobParameterCount : 0)
    readonly property real parameterExtraWidth: Math.max(0, Math.min(parametersFlickable.width - parameterMinimumWidth, activeButtonParameterCount * 5 + activeKnobParameterCount * 20))
    readonly property real parameterInitialGrowth: activeButtonParameterCount + activeKnobParameterCount > 0 ? Math.min(5, parameterExtraWidth / (activeButtonParameterCount + activeKnobParameterCount)) : 0
    readonly property real parameterMinimumWidth: activeButtonParameterCount * 55 + activeKnobParameterCount * 40
    readonly property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(unitNumber, effectNumber)
    required property color unitColor
    readonly property color unitDimColor: unitNumber < 3 ? LateNightTheme.effectsUnitDimColor12 : LateNightTheme.effectsUnitDimColor34
    required property string unitGroup
    required property int unitNumber

    function recountActiveParameters() {
        let activeButtonCount = 0;
        for (let buttonIndex = 0; buttonIndex < buttonRepeater.count; ++buttonIndex) {
            const loader = buttonRepeater.itemAt(buttonIndex);
            if (loader && loader.active) {
                ++activeButtonCount;
            }
        }
        let activeKnobCount = 0;
        for (let knobIndex = 0; knobIndex < knobRepeater.count; ++knobIndex) {
            const loader = knobRepeater.itemAt(knobIndex);
            if (loader && loader.active) {
                ++activeKnobCount;
            }
        }
        activeButtonParameterCount = activeButtonCount;
        activeKnobParameterCount = activeKnobCount;
    }

    implicitHeight: expanded ? 51 : 34

    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: slotControls.left
        anchors.top: parent.top
        border.color: root.focused ? LateNightTheme.effectsFocusBorderColor : root.focusInactiveBorderColor
        border.width: 1
        color: LateNightTheme.effectsParameterPanelColor
        visible: root.expanded

        Image {
            anchors.fill: parent
            anchors.margins: 1
            fillMode: Image.Tile
            source: LateNightTheme.optionalDeckControlsBackgroundTile
            visible: LateNightTheme.isClassic
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            color: "#32000000"
            visible: root.focused
        }
    }
    Item {
        id: slotControls

        anchors.right: parent.right
        height: 30
        width: root.expanded ? 182 : parent.width
        y: Math.round((parent.height - height) / 2)

        EffectFocusButton {
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            effectNumber: root.effectNumber
            height: 16
            unitGroup: root.unitGroup
            visible: showFocus.value > 0
            width: 16
        }
        EffectControlButton {
            id: enableButton

            activeBackgroundSource: LateNightTheme.assetFxSlotButtonActiveBackground
            activeColor: root.unitColor
            activeSource: LateNightTheme.assetFxToggleActiveButton
            anchors.left: parent.left
            anchors.leftMargin: showFocus.value > 0 ? 20 : 2
            anchors.verticalCenter: parent.verticalCenter
            backgroundSource: LateNightTheme.assetFxSlotButtonBackground
            group: root.slot.group
            height: 26
            key: "enabled"
            normalColor: LateNightTheme.effectsSlotToggleInactiveColor
            normalSource: LateNightTheme.assetFxToggleButton
            width: 26
        }
        LateNightControls.Knob {
            id: metaKnob

            anchors.left: enableButton.right
            anchors.leftMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            backgroundSource: LateNightTheme.assetSmallKnobBackground
            displayArc: true
            displayArcColor: root.unitColor
            displayArcRadius: 11.5
            displayArcStart: LateNightControls.Knob.ArcStart.Minimum
            group: root.slot.group
            height: 30
            indicatorColor: root.unitNumber < 3 ? "green" : "blue"
            indicatorKind: "small"
            key: "meta"
            width: 35
        }
        EffectSelector {
            id: effectSelector

            anchors.left: metaKnob.right
            anchors.leftMargin: 2
            anchors.right: parent.right
            anchors.rightMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            height: 24
            slot: root.slot
        }
    }
    Flickable {
        id: parametersFlickable

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1
        anchors.left: parent.left
        anchors.right: slotControls.left
        anchors.top: parent.top
        anchors.topMargin: 3
        clip: true
        contentHeight: height
        contentWidth: Math.max(width, parameterRow.implicitWidth)
        flickableDirection: Flickable.HorizontalFlick
        visible: root.expanded

        Row {
            id: parameterRow

            height: parent.height
            width: implicitWidth
            x: Math.max(0, parametersFlickable.width - width)

            Repeater {
                id: buttonRepeater

                model: root.slot.parametersModel

                delegate: Loader {
                    id: buttonLoader

                    property bool completed: false
                    required property string controlKey
                    required property bool loaded
                    required property string name
                    required property string shortName
                    required property int type
                    required property string unitString

                    function updateActive() {
                        if (completed) {
                            active = loaded && controlKey.length > 0 && type === 1;
                            Qt.callLater(root.recountActiveParameters);
                        }
                    }

                    active: false
                    height: active && item ? item.implicitHeight : 0
                    visible: active
                    width: active && item ? root.buttonParameterWidth : 0

                    sourceComponent: EffectParameter {
                        buttonParameter: true
                        controlKey: buttonLoader.controlKey
                        group: root.slot.group
                        label: buttonLoader.shortName || buttonLoader.name
                        linkColor: root.unitDimColor
                        unitColor: root.unitColor
                        unitString: buttonLoader.unitString
                    }

                    Component.onCompleted: {
                        completed = true;
                        updateActive();
                    }
                    onControlKeyChanged: updateActive()
                    onLoadedChanged: updateActive()
                    onTypeChanged: updateActive()
                }

                onCountChanged: Qt.callLater(root.recountActiveParameters)
            }
            Repeater {
                id: knobRepeater

                model: root.slot.parametersModel

                delegate: Loader {
                    id: knobLoader

                    property bool completed: false
                    required property string controlKey
                    required property bool loaded
                    required property string name
                    required property string shortName
                    required property int type
                    required property string unitString

                    function updateActive() {
                        if (completed) {
                            active = loaded && controlKey.length > 0 && type === 0;
                            Qt.callLater(root.recountActiveParameters);
                        }
                    }

                    active: false
                    height: active && item ? item.implicitHeight : 0
                    visible: active
                    width: active && item ? root.knobParameterWidth : 0

                    sourceComponent: EffectParameter {
                        buttonParameter: false
                        controlKey: knobLoader.controlKey
                        group: root.slot.group
                        label: knobLoader.shortName || knobLoader.name
                        linkColor: root.unitDimColor
                        unitColor: root.unitColor
                        unitString: knobLoader.unitString
                    }

                    Component.onCompleted: {
                        completed = true;
                        updateActive();
                    }
                    onControlKeyChanged: updateActive()
                    onLoadedChanged: updateActive()
                    onTypeChanged: updateActive()
                }

                onCountChanged: Qt.callLater(root.recountActiveParameters)
            }
        }
    }
    Mixxx.ControlProxy {
        id: focusedEffect

        group: root.unitGroup
        key: "focused_effect"
    }
    Mixxx.ControlProxy {
        id: showFocus

        group: root.unitGroup
        key: "show_focus"
    }
}
