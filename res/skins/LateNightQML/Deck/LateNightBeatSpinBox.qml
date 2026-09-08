import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group
    required property string key
    property string decrementKey: ""
    property string incrementKey: ""
    property int preferredWidth: 78
    property bool handlingEditResult: false
    property bool valueInputDirty: false
    readonly property bool useStepControls: decrementKey.length > 0 && incrementKey.length > 0
    readonly property var beatSizes: [
        1 / 32,
        1 / 16,
        1 / 8,
        1 / 4,
        1 / 2,
        1,
        2,
        4,
        8,
        16,
        32,
        64
    ]
    readonly property var beatFractions: [
        [1, 2],
        [1, 4],
        [3, 4],
        [1, 3],
        [2, 3],
        [1, 8],
        [3, 8],
        [5, 8],
        [7, 8],
        [1, 16],
        [3, 16],
        [5, 16],
        [7, 16],
        [9, 16],
        [11, 16],
        [13, 16],
        [15, 16],
        [1, 32],
        [3, 32],
        [5, 32],
        [7, 32],
        [9, 32],
        [11, 32],
        [13, 32],
        [15, 32],
        [17, 32],
        [19, 32],
        [21, 32],
        [23, 32],
        [25, 32],
        [27, 32],
        [29, 32],
        [31, 32]
    ]
    readonly property string valueText: formatBeatSize(valueProxy.value)

    implicitWidth: preferredWidth
    implicitHeight: 26

    function formatBeatSize(value) {
        if (!isFinite(value) || value <= 0) {
            return "";
        }

        if (value >= 1 && Math.abs(value - Math.round(value)) < 0.0001) {
            return Math.round(value).toString();
        }

        var wholePart = Math.floor(value);
        var fractionalPart = value - wholePart;
        for (var i = 0; i < beatFractions.length; ++i) {
            var numerator = beatFractions[i][0];
            var denominator = beatFractions[i][1];
            if (Math.abs(fractionalPart - numerator / denominator) < 0.0001) {
                var fractionText = numerator + "/" + denominator;
                return wholePart > 0 ? wholePart + " " + fractionText : fractionText;
            }
        }

        return value.toString();
    }

    function parseBeatSize(text) {
        var trimmedText = text.trim();
        var mixedParts = trimmedText.match(
                /^([0-9]+)\s+([0-9]+)\s*\/\s*([0-9]+)$/);
        if (mixedParts) {
            var wholePart = Number(mixedParts[1]);
            var mixedNumerator = Number(mixedParts[2]);
            var mixedDenominator = Number(mixedParts[3]);
            if (mixedNumerator > 0 && mixedDenominator > 0) {
                return wholePart + mixedNumerator / mixedDenominator;
            }
            return valueProxy.value;
        }

        var fractionParts = trimmedText.match(/^([0-9]+)\s*\/\s*([0-9]+)$/);
        if (fractionParts) {
            var numerator = Number(fractionParts[1]);
            var denominator = Number(fractionParts[2]);
            if (numerator > 0 && denominator > 0) {
                return numerator / denominator;
            }
            return valueProxy.value;
        }

        var parsed = Number(trimmedText);
        return isFinite(parsed) && parsed > 0 ? parsed : valueProxy.value;
    }

    function nearestBeatSizeIndex(value) {
        var nearestIndex = 0;
        var nearestDistance = Math.abs(beatSizes[0] - value);
        for (var i = 1; i < beatSizes.length; ++i) {
            var distance = Math.abs(beatSizes[i] - value);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestIndex = i;
            }
        }
        return nearestIndex;
    }

    function focusAndSelectValue() {
        valueInput.forceActiveFocus();
        valueInput.selectAll();
    }

    function step(delta) {
        var nextValue;
        if (root.useStepControls) {
            if (delta < 0) {
                decrementControl.trigger();
            } else {
                incrementControl.trigger();
            }
            root.valueInputDirty = false;
            valueInput.text = root.valueText;
            root.focusAndSelectValue();
            return;
        }
        var index = nearestBeatSizeIndex(valueProxy.value);
        index = Math.max(0, Math.min(beatSizes.length - 1, index + delta));
        nextValue = beatSizes[index];
        valueProxy.value = nextValue;
        root.valueInputDirty = false;
        valueInput.text = root.formatBeatSize(nextValue);
        root.focusAndSelectValue();
    }

    function commitText() {
        if (root.handlingEditResult) {
            return;
        }

        root.handlingEditResult = true;
        var parsedValue = parseBeatSize(valueInput.text);
        if (isFinite(parsedValue) && parsedValue > 0) {
            valueProxy.value = parsedValue;
        }
        root.valueInputDirty = false;
        valueInput.text = root.valueText;
        valueInput.focus = false;
        root.handlingEditResult = false;
    }

    function cancelText() {
        if (root.handlingEditResult) {
            return;
        }

        root.handlingEditResult = true;
        root.valueInputDirty = false;
        valueInput.text = root.valueText;
        valueInput.focus = false;
        root.handlingEditResult = false;
    }

    Mixxx.ControlProxy {
        id: valueProxy

        group: root.group
        key: root.key
    }

    Mixxx.ControlProxy {
        id: decrementControl

        group: root.group
        key: root.decrementKey.length > 0 ? root.decrementKey : root.key
    }

    Mixxx.ControlProxy {
        id: incrementControl

        group: root.group
        key: root.incrementKey.length > 0 ? root.incrementKey : root.key
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: "#0f0f0f"
    }

    BorderImage {
        anchors.fill: parent
        source: valueInput.activeFocus
                ? LateNightTheme.assetDeckBeatSpinBoxFocusBorder
                : LateNightTheme.assetDeckBeatSpinBoxBorder
        border {
            top: 2
            left: 2
            right: 19
            bottom: 2
        }
        opacity: 1.0
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 3
        anchors.rightMargin: 0
        anchors.bottomMargin: 2
        spacing: 0

        TextInput {
            id: valueInput

            Layout.fillWidth: true
            text: root.valueText
            font.family: "Open Sans"
            font.pixelSize: 13
            font.bold: true
            color: LateNightTheme.deckBeatSpinBoxTextColor
            selectedTextColor: valueInput.activeFocus
                                ? LateNightTheme.deckBeatSpinBoxFocusedSelectedTextColor
                                : LateNightTheme.deckBeatSpinBoxSelectedTextColor
            selectionColor: valueInput.activeFocus
                            ? LateNightTheme.deckBeatSpinBoxFocusedSelectionColor
                            : LateNightTheme.deckBeatSpinBoxSelectionColor
            horizontalAlignment: TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            selectByMouse: true
            clip: true

            onAccepted: root.commitText()
            onTextEdited: root.valueInputDirty = true
            Keys.onEscapePressed: event => {
                root.cancelText();
                event.accepted = true;
            }
            onActiveFocusChanged: {
                if (!activeFocus && !root.handlingEditResult) {
                    root.commitText();
                }
            }

            Connections {
                function onValueChanged() {
                    if (!valueInput.activeFocus || !root.valueInputDirty) {
                        valueInput.text = root.valueText;
                        root.valueInputDirty = false;
                    }
                }

                target: valueProxy
            }
        }

        Column {
            Layout.preferredWidth: 17
            Layout.alignment: Qt.AlignVCenter
            spacing: 0

            Item {
                width: parent.width
                height: 11

                Image {
                    anchors.fill: parent
                    source: LateNightTheme.assetDeckBeatSpinBoxUpButton
                    fillMode: Image.PreserveAspectFit
                    visible: source.toString().length > 0
                }

                Text {
                    anchors.fill: parent
                    text: "▲"
                    font.pixelSize: 8
                    color: LateNightTheme.textColorMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: LateNightTheme.assetDeckBeatSpinBoxUpButton.toString().length <= 0
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.step(1)
                }
            }

            Item {
                width: parent.width
                height: 11

                Image {
                    anchors.fill: parent
                    source: LateNightTheme.assetDeckBeatSpinBoxDownButton
                    fillMode: Image.PreserveAspectFit
                    visible: source.toString().length > 0
                }

                Text {
                    anchors.fill: parent
                    text: "▼"
                    font.pixelSize: 8
                    color: LateNightTheme.textColorMuted
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: LateNightTheme.assetDeckBeatSpinBoxDownButton.toString().length <= 0
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.step(-1)
                }
            }
        }
    }
}
