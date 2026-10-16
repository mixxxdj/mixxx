pragma ComponentBehavior: Bound

import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    property string mode: "listen"
    property real originalEditValue: 0
    property bool editingBpm: true

    readonly property bool listenHovered: listenArea.containsMouse
    readonly property int hoverLeaveTimeout: 2000
    readonly property int inactiveTimeout: 5000
    readonly property int tapModeClickGuardTimeout: 350
    readonly property real bpmStepSize: 1.0
    readonly property real rateStepSize: 0.01
    readonly property bool trackLoaded: trackLoadedProxy.value > 0

    function switchMode(newMode) {
        hideTimer.stop();
        if (!root.trackLoaded) {
            newMode = "listen";
        }
        root.mode = newMode;
        if (newMode === "select") {
            hideTimer.stop();
        } else if (newMode === "edit") {
            startEditMode();
        } else if (newMode === "tap") {
            tapClickGuardTimer.restart();
            hideTimer.stop();
        } else if (newMode === "listen") {
            editInput.text = "";
            editInput.focus = false;
        }
    }

    function handleTapClick(mouse) {
        if (tapClickGuardTimer.running) {
            return;
        }
        root.tapControl(mouse.button === Qt.RightButton ? bpmTapProxy : tempoTapProxy);
    }

    function tapControl(control) {
        hideTimer.interval = root.inactiveTimeout;
        hideTimer.restart();
        control.value = 1;
        control.value = 0;
    }

    function startEditMode() {
        root.editingBpm = fileBpmProxy.value !== 0;
        if (root.editingBpm) {
            root.originalEditValue = bpmProxy.value;
            editInput.text = root.originalEditValue.toFixed(2);
        } else {
            root.originalEditValue = rateRatioProxy.value * 100;
            editInput.text = root.originalEditValue.toFixed(2);
        }
        editInput.forceActiveFocus();
        editInput.selectAll();
    }

    function applyStep(steps) {
        if (root.editingBpm) {
            const newBpm = Math.max(1, Math.min(fileBpmProxy.value * 4, bpmProxy.value + steps * root.bpmStepSize));
            bpmProxy.value = newBpm;
            editInput.text = newBpm.toFixed(2);
            root.originalEditValue = newBpm;
        } else {
            const newRateRatio = Math.max(0.01, Math.min(4.0, rateRatioProxy.value + steps * root.rateStepSize));
            rateRatioProxy.value = newRateRatio;
            editInput.text = (newRateRatio * 100).toFixed(2);
            root.originalEditValue = newRateRatio * 100;
        }
        editInput.selectAll();
    }

    function applyEditValueAndQuit() {
        const input = editInput.text.trim();
        if (input.length === 0) {
            switchMode("listen");
            return;
        }

        const parsedValue = Number(input);
        if (!Number.isFinite(parsedValue)) {
            switchMode("listen");
            return;
        }
        if (parsedValue !== root.originalEditValue) {
            if (root.editingBpm) {
                bpmProxy.value = Math.max(1, Math.min(fileBpmProxy.value * 4, parsedValue));
            } else {
                rateRatioProxy.value = Math.max(0.01, Math.min(4.0, parsedValue / 100));
            }
        }
        switchMode("listen");
    }

    Mixxx.ControlProxy {
        id: tempoTapProxy
        group: root.group
        key: "tempo_tap"
    }

    Mixxx.ControlProxy {
        id: bpmTapProxy
        group: root.group
        key: "bpm_tap"
    }

    Mixxx.ControlProxy {
        id: trackLoadedProxy
        group: root.group
        key: "track_loaded"
    }

    Mixxx.ControlProxy {
        id: bpmProxy
        group: root.group
        key: "bpm"
    }

    Mixxx.ControlProxy {
        id: fileBpmProxy
        group: root.group
        key: "file_bpm"
    }

    Mixxx.ControlProxy {
        id: rateRatioProxy
        group: root.group
        key: "rate_ratio"
    }

    onTrackLoadedChanged: {
        if (!root.trackLoaded) {
            root.switchMode("listen");
        }
    }

    Timer {
        id: hideTimer
        interval: root.hoverLeaveTimeout
        repeat: false
        onTriggered: {
            if (root.mode !== "edit") {
                root.switchMode("listen");
            }
        }
    }

    Timer {
        id: tapClickGuardTimer
        interval: root.tapModeClickGuardTimeout
        repeat: false
    }

    MouseArea {
        id: listenArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        visible: root.mode === "listen"
        onClicked: root.switchMode("select")
        onDoubleClicked: function(mouse) {
            root.switchMode(mouse.x < width / 2 ? "tap" : "edit");
        }
    }

    Row {
        anchors.fill: parent
        visible: root.mode === "select"

        Rectangle {
            width: parent.width / 2
            height: parent.height
            color: LateNightTheme.bpmTapEditorSelectBackgroundColor
            border.width: 1
            border.color: LateNightTheme.bpmTapEditorSelectBorderColor

            Image {
                anchors.centerIn: parent
                source: LateNightTheme.assetDeckBpmSelectTapButton
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: hideTimer.stop()
                onExited: {
                    if (root.mode === "select") {
                        hideTimer.interval = root.hoverLeaveTimeout;
                        hideTimer.restart();
                    }
                }
                onClicked: root.switchMode("tap")
                onDoubleClicked: root.switchMode("tap")
            }
        }

        Rectangle {
            width: parent.width / 2
            height: parent.height
            color: LateNightTheme.bpmTapEditorSelectBackgroundColor
            border.width: 1
            border.color: LateNightTheme.bpmTapEditorSelectBorderColor

            Image {
                anchors.centerIn: parent
                source: LateNightTheme.assetDeckBpmSelectEditButton
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: hideTimer.stop()
                onExited: {
                    if (root.mode === "select") {
                        hideTimer.interval = root.hoverLeaveTimeout;
                        hideTimer.restart();
                    }
                }
                onClicked: root.switchMode("edit")
                onDoubleClicked: root.switchMode("edit")
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: root.mode === "tap"
        color: LateNightTheme.bpmTapEditorBackgroundColor
        border.width: 1
        border.color: tapArea.pressed ? LateNightTheme.bpmTapEditorEditBorderColor : LateNightTheme.bpmTapEditorSelectBorderColor
        radius: 1

        MouseArea {
            id: tapArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onEntered: hideTimer.stop()
            onExited: {
                hideTimer.interval = root.hoverLeaveTimeout;
                hideTimer.restart();
            }
            onClicked: function(mouse) { root.handleTapClick(mouse); }
            onDoubleClicked: function(mouse) { mouse.accepted = true; }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: root.mode === "edit"
        color: LateNightTheme.bpmTapEditorBackgroundColor
        radius: 1

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: parent.height - 16
            color: LateNightTheme.bpmTapEditorBackgroundColor
            border.width: 1
            border.color: LateNightTheme.bpmTapEditorEditBorderColor
            radius: 1

            TextInput {
                id: editInput
                anchors.fill: parent
                anchors.margins: 1
                color: LateNightTheme.textColor
                selectedTextColor: LateNightTheme.deckActiveButtonTextColor
                selectionColor: LateNightTheme.textColor
                font.family: "Open Sans"
                font.pixelSize: 14
                font.weight: Font.Medium
                horizontalAlignment: TextInput.AlignHCenter
                verticalAlignment: TextInput.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                selectByMouse: true

                Keys.onReturnPressed: root.applyEditValueAndQuit()
                Keys.onEnterPressed: root.applyEditValueAndQuit()
                Keys.onEscapePressed: root.switchMode("listen")
            }
        }

        Rectangle {
            x: 0
            y: parent.height - 16
            width: parent.width / 2
            height: 16
            color: LateNightTheme.bpmTapEditorButtonColor

            Image {
                anchors.fill: parent
                source: decreaseArea.pressed ? LateNightTheme.assetDeckBpmSpinboxMinusPressedButton : LateNightTheme.assetDeckBpmSpinboxMinusButton
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: decreaseArea
                anchors.fill: parent
                onClicked: root.applyStep(-1)
            }
        }

        Rectangle {
            x: parent.width / 2
            y: parent.height - 16
            width: parent.width / 2
            height: 16
            color: LateNightTheme.bpmTapEditorButtonColor

            Image {
                anchors.fill: parent
                source: increaseArea.pressed ? LateNightTheme.assetDeckBpmSpinboxPlusPressedButton : LateNightTheme.assetDeckBpmSpinboxPlusButton
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: increaseArea
                anchors.fill: parent
                onClicked: root.applyStep(1)
            }
        }
    }
}
