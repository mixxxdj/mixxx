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
    readonly property string valueText: formatBeatSize(valueProxy.value)

    implicitWidth: preferredWidth
    implicitHeight: 26

    function formatBeatSize(value) {
        if (value <= 0) {
            return "";
        }
        if (value < 1) {
            return "1/" + Math.round(1 / value);
        }
        if (Math.abs(value - Math.round(value)) < 0.0001) {
            return Math.round(value).toString();
        }
        return value.toString();
    }

    function parseBeatSize(text) {
        var parts = text.split("/");
        if (parts.length === 2) {
            var numerator = Number(parts[0]);
            var denominator = Number(parts[1]);
            if (numerator > 0 && denominator > 0) {
                return numerator / denominator;
            }
            return valueProxy.value;
        }
        var parsed = Number(text);
        return parsed > 0 ? parsed : valueProxy.value;
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

    function step(delta) {
        if (root.useStepControls) {
            if (delta < 0) {
                decrementControl.trigger();
            } else {
                incrementControl.trigger();
            }
            return;
        }
        var index = nearestBeatSizeIndex(valueProxy.value);
        index = Math.max(0, Math.min(beatSizes.length - 1, index + delta));
        valueProxy.value = beatSizes[index];
    }

    function commitText() {
        valueProxy.value = parseBeatSize(valueInput.text);
        valueInput.text = root.valueText;
        valueInput.focus = false;
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
        source: LateNightTheme.assetDeckBeatSpinBoxBorder
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
            horizontalAlignment: TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            selectByMouse: true
            clip: true

            onAccepted: root.commitText()
            onActiveFocusChanged: {
                if (!activeFocus) {
                    text = root.valueText;
                }
            }

            Connections {
                function onValueChanged() {
                    if (!valueInput.activeFocus) {
                        valueInput.text = root.valueText;
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
