import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property bool buttonParameter
    required property string controlKey
    required property string group
    required property string label
    required property color linkColor
    property bool showParameterValue: false
    property bool skipNextValueChange: true
    required property color unitColor
    required property string unitString

    function formatParameterValue(value) {
        const absoluteRoundedValue = Math.abs(Math.round(value));
        const decimalPlaces = absoluteRoundedValue < 100 ? 2 : (absoluteRoundedValue < 1000 ? 1 : 0);
        const decimalFactor = Math.pow(10, decimalPlaces);
        const roundedValue = Math.round(value * decimalFactor) / decimalFactor;
        const displayValue = roundedValue === 0 ? 0 : roundedValue;
        return displayValue.toString() + (root.unitString.length > 0 ? " " + root.unitString : "");
    }
    function resetValueDisplay() {
        valueDisplayTimer.stop();
        root.showParameterValue = false;
        root.skipNextValueChange = true;
    }

    implicitHeight: 45
    implicitWidth: buttonParameter ? 55 : 40

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
        y: 5
    }
    LateNightControls.Knob {
        anchors.horizontalCenter: parent.horizontalCenter
        backgroundSource: LateNightTheme.assetFxKnobBackground
        displayArc: true
        displayArcColor: LateNightTheme.effectsParameterArcColor
        displayArcRadius: 12
        displayArcStart: LateNightControls.Knob.ArcStart.Minimum
        group: root.group
        height: 26
        indicatorColor: LateNightTheme.effectsParameterIndicatorColor
        indicatorKind: "fx"
        key: root.controlKey
        visible: !root.buttonParameter
        width: 26
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        color: LateNightTheme.effectsParameterTextColor
        elide: Text.ElideRight
        font.pixelSize: 10
        height: 10
        horizontalAlignment: Text.AlignHCenter
        text: root.showParameterValue ? root.formatParameterValue(parameterValue.value) : root.label
        verticalAlignment: Text.AlignVCenter
        width: root.width
        y: 28
    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        height: 7
        spacing: 1
        visible: !root.buttonParameter
        y: 38

        Rectangle {
            color: inverseControl.item && inverseControl.item.value > 0 ? LateNightTheme.effectsParameterInverseActiveColor : LateNightTheme.effectsParameterLinkInactiveColor
            height: 7
            radius: 3
            width: 8

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

            readonly property color backgroundColor: LateNightTheme.effectsParameterLinkInactiveColor
            readonly property color leftColor: state === 1 || state === 2 || state === 4 ? root.linkColor : backgroundColor
            readonly property color middleColor: state === 1 ? root.linkColor : backgroundColor
            readonly property color rightColor: state === 1 || state === 3 || state === 4 ? root.linkColor : backgroundColor
            readonly property int state: linkControl.item ? Math.round(linkControl.item.value) : 0

            height: 7
            radius: 3
            width: 34

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
