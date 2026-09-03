pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Popup {
    id: root

    required property string group
    required property int hotcueNumber

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property var currentTrack: deckPlayer?.currentTrack
    property var popupTrack: null
    property int cueRevision: 0
    property int jumpDirectionRevision: 0
    readonly property string hotcueLabel: root.getHotcueLabel(root.cueRevision)
    readonly property color selectedColor: colorProxy.value >= 0
            ? "#" + colorProxy.value.toString(16).padStart(6, "0")
            : LateNightTheme.accentColor
    readonly property string savedJumpDirection:
        root.getSavedJumpDirection(root.jumpDirectionRevision)

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    dim: false
    focus: true
    modal: true
    padding: 0
    width: 224
    height: 248

    function formatHotcueTime() {
        if (trackSamplesProxy.value <= 0 ||
                hotcuePositionProxy.value < 0 ||
                durationProxy.value <= 0) {
            return "";
        }
        var seconds = durationProxy.value * hotcuePositionProxy.value / trackSamplesProxy.value;
        var minutes = Math.floor(seconds / 60);
        var remainingSeconds = seconds - minutes * 60;
        return minutes + ":" + remainingSeconds.toFixed(2).padStart(5, "0");
    }

    function getHotcueLabel(revision) {
        if (revision < 0 || !popupTrack || root.hotcueNumber <= 0) {
            return "";
        }
        return Mixxx.Library.deckHotcueLabel(popupTrack, root.hotcueNumber);
    }

    function getSavedJumpDirection(revision) {
        if (revision < 0 || !popupTrack || root.hotcueNumber <= 0) {
            return "impossible";
        }
        return Mixxx.Library.deckHotcueJumpDirection(popupTrack,
                root.group,
                root.hotcueNumber);
    }

    function setHotcueColor(newColor) {
        colorProxy.value = (parseInt(newColor.r * 255) << 16) |
                (parseInt(newColor.g * 255) << 8) |
                parseInt(newColor.b * 255);
        root.close();
    }

    function triggerMomentary(proxy) {
        proxy.value = 1;
        proxy.value = 0;
    }

    function setCueType(action) {
        if (popupTrack && root.hotcueNumber > 0 && Mixxx.Library.setDeckHotcueType(popupTrack,
                root.group,
                root.hotcueNumber,
                action)) {
            root.cueRevision += 1;
            root.jumpDirectionRevision += 1;
        }
    }

    onOpened: {
        popupTrack = currentTrack;
        labelInput.text = root.hotcueLabel;
        labelInput.forceActiveFocus(Qt.PopupFocusReason);
        labelInput.selectAll();
    }

    onClosed: {
        if (popupTrack && root.hotcueNumber > 0) {
            Mixxx.Library.cleanupDeckHotcuePopup(popupTrack, root.hotcueNumber);
        }
        popupTrack = null;
    }

    Connections {
        function onTrackChanged() {
            if (root.opened) {
                root.close();
            }
        }

        target: root.deckPlayer
    }

    Connections {
        function onCuesChanged() {
            root.cueRevision += 1;
            root.jumpDirectionRevision += 1;
            if (!labelInput.activeFocus) {
                labelInput.text = root.hotcueLabel;
            }
        }

        target: root.popupTrack
    }

    Mixxx.ControlProxy {
        id: colorProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_color"
    }

    Mixxx.ControlProxy {
        id: hotcuePositionProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_position"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: hotcueEndPositionProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_endposition"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: trackSamplesProxy

        group: root.group
        key: "track_samples"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: typeProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_type"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: playPositionProxy

        group: root.group
        key: "playposition"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: quantizeProxy

        group: root.group
        key: "quantize"

        onValueChanged: root.jumpDirectionRevision += 1
    }

    Mixxx.ControlProxy {
        id: clearProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_clear"
    }

    Mixxx.ControlProxy {
        id: durationProxy

        group: root.group
        key: "duration"
    }

    background: Rectangle {
        color: "#141416"
        border.color: "#4b4b4b"
        border.width: 2
    }

    contentItem: FocusScope {
        anchors.fill: parent
        focus: true

        Text {
            id: titleText

            x: 20
            y: 22
            width: 80
            height: 17
            text: "Hotcue #" + root.hotcueNumber
            font.family: "Open Sans"
            font.pixelSize: 14
            color: LateNightTheme.primaryDeckTextColor
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            x: 110
            y: 22
            width: 46
            height: 17
            text: root.formatHotcueTime()
            font.family: "Open Sans"
            font.pixelSize: 14
            color: LateNightTheme.primaryDeckTextColor
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        TextField {
            id: labelInput

            x: 20
            y: 52
            width: 137
            height: 25
            focus: true
            text: root.hotcueLabel
            placeholderText: "Label..."
            placeholderTextColor: "#77716c"
            selectByMouse: true
            color: LateNightTheme.primaryDeckTextColor
            font.family: "Open Sans"
            font.pixelSize: 13
            verticalAlignment: TextInput.AlignVCenter
            leftPadding: 4
            clip: true

            onTextEdited: {
                if (root.popupTrack && root.hotcueNumber > 0) {
                    Mixxx.Library.setDeckHotcueLabel(root.popupTrack,
                            root.hotcueNumber,
                            text);
                }
            }

            Keys.onReturnPressed: root.close()
            Keys.onEnterPressed: root.close()
            Keys.onEscapePressed: root.close()

            background: Rectangle {
                color: "#030303"
                border.color: LateNightTheme.isClassic ? "#f0bb2b" : "#2e2e2e"
                border.width: LateNightTheme.isClassic ? 1 : 0
            }
        }

        GridLayout {
            x: 20
            y: 84
            width: 137
            height: 138
            columns: 3
            rowSpacing: 12
            columnSpacing: 11

            Repeater {
                model: Mixxx.Config.hotcueColorPalette

                delegate: Item {
                    required property color modelData

                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 38

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 1
                        color: parent.modelData
                    }

                    BorderImage {
                        anchors.fill: parent
                        source: LateNightTheme.isPaleMoon ? LateNightTheme.lateNightButton(root.selectedColor.toString().toLowerCase() === parent.modelData.toString().toLowerCase()
                                && LateNightTheme.isPaleMoon ? "btn_colorpicker_active.svg" : "btn_colorpicker.svg")
                                : ""
                        border {
                            top: 2
                            bottom: 2
                            left: 2
                            right: 2
                        }
                        visible: LateNightTheme.isPaleMoon
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border.color: "#060606"
                        border.width: 1
                        visible: LateNightTheme.isClassic
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        font.pixelSize: 23
                        font.bold: true
                        color: "white"
                        style: Text.Outline
                        styleColor: "#222222"
                        visible: parent.modelData.toString().toLowerCase() === root.selectedColor.toString().toLowerCase()
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.setHotcueColor(parent.modelData)
                    }
                }
            }
        }

        LateNightCueMenuButton {
            x: 176
            y: 19
            deleteButton: true
            iconSize: 18
            iconSource: LateNightTheme.lateNightButton("btn__delete.svg")
            onClicked: {
                root.triggerMomentary(clearProxy);
                root.close();
            }
        }

        LateNightCueMenuButton {
            x: 176
            y: 60
            checked: Math.round(typeProxy.value) === 1
            iconSource: LateNightTheme.lateNightButton("btn__beats_hotcues_later.svg")
            activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            onClicked: root.setCueType("standard")
        }

        LateNightCueMenuButton {
            x: 176
            y: 122
            checked: Math.round(typeProxy.value) === 4
            iconSource: LateNightTheme.lateNightButton("btn__loop.svg")
            activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            onClicked: root.setCueType("loop-auto")
            onRightClicked: root.setCueType("loop-manual")
        }

        LateNightCueMenuButton {
            readonly property bool forwardJump: root.savedJumpDirection === "forward"

            x: 176
            y: 183
            checked: Math.round(typeProxy.value) === 5
            impossible: root.savedJumpDirection === "impossible"
            iconSource: LateNightTheme.lateNightButton(forwardJump ? "btn__beatjump_right.svg" : "btn__beatjump_left.svg")
            activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            onClicked: root.setCueType("jump-auto")
            onRightClicked: root.setCueType("jump-manual")
        }
    }
}
