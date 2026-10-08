import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property bool buttonParameter
    required property string controlKey
    required property Mixxx.EffectSlotProxy effectSlot
    required property string group
    required property string label
    required property color linkColor
    required property real maximum
    required property real neutralPoint
    required property int parameterType
    required property Mixxx.EffectSlotParametersModel parametersModel
    property bool showParameterValue: false
    property bool skipNextValueChange: true
    required property color unitColor
    required property string unitString
    required property bool useApplicationFont
    readonly property int parameterSlotNumber: {
        const slotNumber = parseInt(root.controlKey.replace(/\D/g, ""));
        return Number.isNaN(slotNumber) ? -1 : slotNumber - 1;
    }

    function draggedParameterSlotNumber(drag) {
        if (!root.effectSlot.loaded || !drag.hasText) {
            return -1;
        }
        const payload = drag.text.split(/\r?\n/);
        if (payload.length !== 3 ||
                payload[0] !== "Mixxx effect parameter " + root.parameterType ||
                payload[1] !== root.effectSlot.uniqueEffectId) {
            return -1;
        }
        const slotNumber = Number(payload[2]);
        return Number.isInteger(slotNumber) && slotNumber >= 0 ? slotNumber : -1;
    }

    function formatParameterValue(value) {
        const absoluteRoundedValue = Math.round(Math.abs(value));
        const decimalPlaces = absoluteRoundedValue < 100 ? 2 : (absoluteRoundedValue < 1000 ? 1 : 0);
        const decimalFactor = Math.pow(10, decimalPlaces);
        const roundedValue = Math.sign(value) * Math.round(Math.abs(value) * decimalFactor) / decimalFactor;
        const displayValue = roundedValue === 0 ? 0 : roundedValue;
        return root.parametersModel.formatNumber(displayValue) + (root.unitString.length > 0 ? " " + root.unitString : "");
    }
    function resetValueDisplay() {
        valueDisplayTimer.stop();
        root.showParameterValue = false;
        root.skipNextValueChange = true;
    }

    implicitHeight: buttonParameter ? 35 : 43
    implicitWidth: Math.max(buttonParameter ? 55 : 42, Math.min(buttonParameter ? 58 : 60, parametersModel.labelWidth(label, maximum, unitString, parameterLabel.font, useApplicationFont)))

    EffectControlButton {
        activeColor: LateNightTheme.effectsParameterActiveColor
        activeSource: LateNightTheme.assetFxParameterActiveButton
        anchors.horizontalCenter: parent.horizontalCenter
        group: root.group
        height: 20
        key: root.controlKey
        normalColor: LateNightTheme.effectsParameterInactiveColor
        normalSource: LateNightTheme.assetFxParameterButton
        visible: root.buttonParameter
        width: 35
        y: Math.floor((root.height - 32) / 2)
    }
    LateNightControls.Knob {
        backgroundSource: LateNightTheme.assetFxKnobBackground
        displayArc: true
        displayArcColor: LateNightTheme.effectsParameterArcColor
        displayArcOffsetY: 0
        displayArcOrigin: root.neutralPoint
        displayArcRadius: 12
        displayArcStart: LateNightControls.Knob.ArcStart.Minimum
        group: root.group
        height: 26
        indicatorColor: LateNightTheme.effectsParameterIndicatorColor
        indicatorKind: "fx"
        key: root.controlKey
        visible: !root.buttonParameter
        width: 26
        x: Math.floor((parent.width - width + 1) / 2)
    }
    Text {
        id: parameterLabel

        color: LateNightTheme.effectsParameterTextColor
        elide: Text.ElideRight
        font.family: "Open Sans"
        font.pixelSize: 10
        font.weight: Font.Medium
        height: 10
        horizontalAlignment: Text.AlignHCenter
        renderType: Text.NativeRendering
        text: root.showParameterValue ? root.formatParameterValue(parameterValue.value) : root.label
        verticalAlignment: Text.AlignVCenter
        width: Math.min(root.width, root.buttonParameter ? 58 : 60)
        y: root.buttonParameter ? Math.floor((root.height - 32) / 2) + 22 : 26

        Rectangle {
            anchors.fill: parent
            color: "#151515"
            visible: LateNightTheme.isClassic
            z: -1
        }

        Drag.active: parameterDragHandler.active
        Drag.dragType: Drag.Automatic
        Drag.mimeData: ({
            "text/plain": "Mixxx effect parameter " + root.parameterType + "\n" +
                    root.effectSlot.uniqueEffectId + "\n" + root.parameterSlotNumber
        })
        Drag.proposedAction: Qt.MoveAction
        Drag.supportedActions: Qt.MoveAction

        Drag.onDragFinished: dropAction => {
            const effectSlot = root.effectSlot;
            Qt.callLater(() => effectSlot.completeParameterSwap(
                                dropAction === Qt.MoveAction));
        }

        DragHandler {
            id: parameterDragHandler

            acceptedButtons: Qt.LeftButton
            enabled: root.effectSlot.loaded
            target: null
        }

        HoverHandler {
            enabled: root.effectSlot.loaded && root.parameterSlotNumber >= 0
            cursorShape: parameterDragHandler.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        }

        DropArea {
            anchors.fill: parent

            onEntered: drag => {
                drag.accepted = root.parameterSlotNumber >= 0 &&
                        root.draggedParameterSlotNumber(drag) >= 0;
            }
            onDropped: drop => {
                const sourceSlotNumber = root.draggedParameterSlotNumber(drop);
                if (sourceSlotNumber < 0) {
                    return;
                }
                root.effectSlot.queueParameterSwap(
                            root.parameterType,
                            root.parameterSlotNumber,
                            sourceSlotNumber);
                drop.acceptProposedAction();
            }
        }
    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        height: 7
        spacing: 1
        visible: !root.buttonParameter
        width: 42
        y: 37

        Rectangle {
            color: inverseControl.item && inverseControl.item.value > 0 ? LateNightTheme.effectsParameterInverseActiveColor : LateNightTheme.effectsParameterLinkInactiveColor
            height: 6
            radius: 3
            width: 7

            TapHandler {
                onTapped: {
                    if (inverseControl.item) {
                        inverseControl.item.value = inverseControl.item.value > 0 ? 0 : 1;
                    }
                }
            }
        }
        Rectangle {
            id: linkBar

            readonly property color backgroundColor: state === 0 ? LateNightTheme.effectsParameterLinkInactiveColor : "#333333"
            readonly property color leftColor: state === 1 || state === 2 || state === 4 ? root.linkColor : backgroundColor
            readonly property color middleColor: state === 1 ? root.linkColor : backgroundColor
            readonly property color rightColor: state === 1 || state === 3 || state === 4 ? root.linkColor : backgroundColor
            readonly property int state: linkControl.item ? Math.round(linkControl.item.value) : 0

            height: 6
            radius: 3
            width: 33

            gradient: Gradient {
                orientation: Gradient.Horizontal

                GradientStop {
                    color: linkBar.leftColor
                    position: 0
                }
                GradientStop {
                    color: linkBar.leftColor
                    position: 0.33
                }
                GradientStop {
                    color: linkBar.middleColor
                    position: 0.34
                }
                GradientStop {
                    color: linkBar.middleColor
                    position: 0.66
                }
                GradientStop {
                    color: linkBar.rightColor
                    position: 0.67
                }
                GradientStop {
                    color: linkBar.rightColor
                    position: 1
                }
            }

            TapHandler {
                onTapped: {
                    if (linkControl.item) {
                        linkControl.item.value = (Math.round(linkControl.item.value) + 1) % 5;
                    }
                }
            }
        }
    }
    Loader {
        id: inverseControl

        active: !root.buttonParameter

        sourceComponent: ParameterControlProxy {
            group: root.group
            key: root.controlKey + "_link_inverse"
        }
    }
    Loader {
        id: linkControl

        active: !root.buttonParameter

        sourceComponent: ParameterControlProxy {
            group: root.group
            key: root.controlKey + "_link_type"
        }
    }
    Timer {
        id: valueDisplayTimer

        interval: 800

        onTriggered: root.showParameterValue = false
    }
    Mixxx.ControlProxy {
        id: parameterValue

        group: root.group
        key: root.controlKey

        onGroupChanged: root.resetValueDisplay()
        onKeyChanged: root.resetValueDisplay()
        onValueChanged: value => {
            if (root.skipNextValueChange) {
                root.skipNextValueChange = false;
                return;
            }
            if (!root.buttonParameter) {
                root.showParameterValue = true;
                valueDisplayTimer.restart();
            }
        }
    }

    component ParameterControlProxy: Item {
        id: proxyRoot

        required property string group
        required property string key
        property alias value: control.value

        Mixxx.ControlProxy {
            id: control

            group: proxyRoot.group
            key: proxyRoot.key
        }
    }
}
