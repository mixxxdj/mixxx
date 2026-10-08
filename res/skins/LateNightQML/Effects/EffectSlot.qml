pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    property int activeButtonParameterCount: 0
    property int activeKnobParameterCount: 0
    readonly property int controlsLeftInset: LateNightTheme.isClassic ? (expanded ? 4 : 5) : 3
    readonly property int controlsRightInset: LateNightTheme.isClassic ? (expanded ? 5 : 7) : (expanded ? 7 : 3)
    required property int effectNumber
    property bool expanded: false
    readonly property bool focused: showFocus.value > 0 && Math.round(focusedEffect.value) === effectNumber
    property bool initialParameterLayout: true
    readonly property int parameterAreaWidth: Math.max(0, width - 182)
    property var parameterData: []
    property real parameterMinimumWidth: 0
    property var parameterWidths: []
    readonly property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(unitNumber, effectNumber)
    readonly property color unitArcColor: unitNumber < 3 ? LateNightTheme.effectsControllerColor12 : LateNightTheme.effectsControllerColor34
    required property color unitColor
    readonly property color unitDimColor: unitNumber < 3 ? LateNightTheme.effectsUnitDimColor12 : LateNightTheme.effectsUnitDimColor34
    required property string unitGroup
    required property int unitNumber

    function recountActiveParameters() {
        const minimums = [];
        let buttons = 0;
        for (let index = 0; index < parameterRepeater.count; ++index) {
            const loader = parameterRepeater.itemAt(index);
            minimums.push(loader && loader.item ? loader.item.implicitWidth : (root.parameterData[index].type === 1 ? 55 : 42));
            buttons += root.parameterData[index].type === 1 ? 1 : 0;
        }
        root.parameterMinimumWidth = minimums.reduce((sum, value) => sum + value, 0);
        root.activeButtonParameterCount = buttons;
        root.activeKnobParameterCount = minimums.length - buttons;
        let remaining = Math.max(root.parameterMinimumWidth, Math.min(parametersContainer.width, minimums.length * 60));
        let count = minimums.length;
        const fixed = minimums.map(() => false);
        for (let changed = true; changed && count > 0; ) {
            changed = false;
            for (let index = 0; index < minimums.length; ++index) {
                if (!fixed[index] && minimums[index] > remaining / count) {
                    fixed[index] = true;
                    remaining -= minimums[index];
                    --count;
                    changed = true;
                }
            }
        }
        let carry = 0;
        root.parameterWidths = minimums.map((minimum, index) => {
            const exact = fixed[index] ? minimum : remaining / count;
            const width = Math.round(exact + carry);
            carry += exact - width;
            return width;
        });
    }
    function refreshParameters() {
        const model = root.slot.parametersModel;
        const parameters = [];
        for (let index = 0; index < model.rowCount(); ++index) {
            const parameter = model.get(index);
            const number = parseInt(parameter.controlKey.replace(/\D/g, ""));
            if (parameter.loaded && number <= 8) {
                const previous = root.parameterData.find(item => item.controlKey === parameter.controlKey);
                parameter.useApplicationFont = root.initialParameterLayout || (previous && previous.parameterId === parameter.parameterId && previous.useApplicationFont) || false;
                parameters.push(parameter);
            }
        }
        parameters.sort((left, right) => right.type - left.type || parseInt(left.controlKey.replace(/\D/g, "")) - parseInt(right.controlKey.replace(/\D/g, "")));
        root.initialParameterLayout = false;
        root.parameterData = parameters;
        Qt.callLater(root.recountActiveParameters);
    }

    implicitHeight: expanded ? (LateNightTheme.isPaleMoon ? 50 : (activeKnobParameterCount > 0 ? 50 : activeButtonParameterCount > 0 ? 42 : 35)) : 34

    Component.onCompleted: refreshParameters()
    onWidthChanged: Qt.callLater(recountActiveParameters)

    Connections {
        function onDataChanged() {
            root.refreshParameters();
        }
        function onModelReset() {
            root.parameterData = [];
            root.refreshParameters();
        }

        target: root.slot.parametersModel
    }
    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }
    LateNightControls.Panel {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        borderVisible: LateNightTheme.isClassic || root.focused
        bottomBorderColor: root.focused ? LateNightTheme.effectsFocusBorderColor : LateNightTheme.deckPanelBorderLight
        color: LateNightTheme.effectsParameterPanelColor
        leftBorderColor: topBorderColor
        rightBorderColor: bottomBorderColor
        topBorderColor: root.focused ? LateNightTheme.effectsFocusBorderColor : LateNightTheme.deckPanelBorderDark
        visible: root.expanded
        width: root.parameterAreaWidth

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
            color: LateNightTheme.isClassic ? "#32000000" : "#32000001"
            visible: root.focused
        }
    }
    Item {
        id: slotControls

        height: LateNightTheme.isClassic ? (root.expanded ? 35 : 33) : 30
        width: Math.max(0, (root.expanded ? Math.min(182, parent.width) : parent.width) - root.controlsLeftInset - root.controlsRightInset)
        x: parent.width - width - root.controlsRightInset
        y: !root.expanded ? 2 : LateNightTheme.isClassic ? Math.floor((parent.height - height) / 2) : Math.round((parent.height - height) / 2)

        EffectFocusButton {
            anchors.left: parent.left
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
            backgroundSource: LateNightTheme.assetFxSlotButtonBackground
            group: root.slot.group
            height: 26
            key: "enabled"
            normalColor: LateNightTheme.effectsSlotToggleInactiveColor
            normalSource: LateNightTheme.assetFxToggleButton
            width: 26
            x: showFocus.value > 0 ? 19 : 0
            y: Math.floor((parent.height - height) / 2)
        }
        Item {
            id: metaKnobFrame

            height: LateNightTheme.isClassic ? 33 : 30
            width: LateNightTheme.isClassic ? 43 : 35
            x: enableButton.x + enableButton.width
            y: root.expanded && LateNightTheme.isClassic ? 1 : 0

            LateNightControls.Knob {
                backgroundSource: LateNightTheme.assetSmallKnobBackground
                displayArc: true
                displayArcColor: root.unitArcColor
                displayArcOffsetY: 1.883
                displayArcOrigin: root.slot.metaDefault
                displayArcRadius: LateNightTheme.mixerArcRadiusCompact
                displayArcStart: LateNightControls.Knob.ArcStart.Minimum
                group: root.slot.group
                height: 30
                indicatorColor: root.unitNumber < 3 ? "green" : "blue"
                indicatorKind: "small"
                key: "meta"
                width: 35
                x: LateNightTheme.isClassic ? 4 : 0
                y: LateNightTheme.isClassic ? 1 : 0
            }
        }
        EffectSelector {
            id: effectSelector

            height: 24
            slot: root.slot
            width: Math.max(0, parent.width - x)
            x: metaKnobFrame.x + metaKnobFrame.width
            y: Math.floor((parent.height - height) / 2)
        }
    }
    Item {
        id: parametersContainer

        anchors.bottom: parent.bottom
        anchors.bottomMargin: LateNightTheme.isClassic ? 3 : 2
        anchors.left: parent.left
        anchors.leftMargin: LateNightTheme.isClassic ? 2 : 0
        anchors.top: parent.top
        anchors.topMargin: LateNightTheme.isClassic ? 4 : 5
        clip: true
        visible: root.expanded
        width: Math.max(0, root.parameterAreaWidth - (LateNightTheme.isClassic ? 4 : 0))

        RowLayout {
            id: parameterRow

            height: parent.height
            spacing: 0
            width: Math.max(root.parameterMinimumWidth, Math.min(parametersContainer.width, (root.activeButtonParameterCount + root.activeKnobParameterCount) * 60))
            x: Math.max(0, parametersContainer.width - width)

            Repeater {
                id: parameterRepeater

                model: root.parameterData

                delegate: Loader {
                    id: parameterLoader

                    required property int index
                    required property var modelData

                    Layout.alignment: Qt.AlignTop
                    Layout.maximumWidth: Layout.minimumWidth
                    Layout.minimumWidth: root.parameterWidths[index] || 0
                    Layout.preferredHeight: parameterRow.height
                    Layout.preferredWidth: Layout.minimumWidth

                    sourceComponent: EffectParameter {
                        buttonParameter: parameterLoader.modelData.type === 1
                        controlKey: parameterLoader.modelData.controlKey
                        effectSlot: root.slot
                        group: root.slot.group
                        label: parameterLoader.modelData.shortName || parameterLoader.modelData.name
                        linkColor: root.unitDimColor
                        maximum: parameterLoader.modelData.maximum
                        neutralPoint: parameterLoader.modelData.neutralPoint
                        parameterType: parameterLoader.modelData.type
                        parametersModel: root.slot.parametersModel
                        unitColor: root.unitColor
                        unitString: parameterLoader.modelData.unitString
                        useApplicationFont: parameterLoader.modelData.useApplicationFont

                        onImplicitWidthChanged: Qt.callLater(root.recountActiveParameters)
                    }

                    onLoaded: Qt.callLater(root.recountActiveParameters)
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
