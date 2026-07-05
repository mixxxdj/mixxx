import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitWidth: 158
    implicitHeight: 20

    Mixxx.ControlProxy {
        id: vinylEnabledControl
        group: root.group
        key: "vinylcontrol_enabled"
    }

    Mixxx.ControlProxy {
        id: vinylStatusProxy
        group: root.group
        key: "vinylcontrol_status"
    }

    Mixxx.ControlProxy {
        id: passthroughControl
        group: root.group
        key: "passthrough"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Vinyl enable/disable toggle with status display.
        // The click requests vinylcontrol_enabled; the fill displays vinylcontrol_status.
        Item {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 20

            LateNightIconButton {
                anchors.fill: parent
                backgroundSource: LateNightTheme.lateNightButton("btn_embedded_library.svg")
                activeBackgroundSuffix: "active"
                activeState: vinylStatusProxy.value > 0
                inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                activeColor: LateNightTheme.vinylStatusColor(vinylStatusProxy.value)
                label: "VINYL"
                labelPixelSize: 11
                labelColor: activeState ? LateNightTheme.deckActiveButtonTextColor : LateNightTheme.textColorMuted
                contentOpacity: activeState ? 0.95 : 0.72
                useBorderImageBackground: true
                backgroundBorderTop: 2
                backgroundBorderBottom: 2
                backgroundBorderLeft: 2
                backgroundBorderRight: 2
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                preventStealing: true

                onClicked: {
                    vinylEnabledControl.parameter = vinylEnabledControl.value > 0 ? 0 : 1;
                }
            }
        }

        // Vinyl mode: cycles ABS(0) → REL(1) → CONST(2) → ABS(0)
        LateNightCycleButton {
            Layout.preferredWidth: 46
            Layout.preferredHeight: 20
            backgroundSource: LateNightTheme.lateNightButton("btn_embedded_grid.svg")
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            inactiveLabelColor: LateNightTheme.textColorMuted
            activeLabelColor: LateNightTheme.textColorMuted
            activeOpacity: 0.72
            inactiveOpacity: 0.72
            labelPixelSize: 11
            useBorderImageBackground: true
            backgroundBorderTop: 2
            backgroundBorderBottom: 2
            backgroundBorderLeft: 1
            backgroundBorderRight: 2
            group: root.group
            key: "vinylcontrol_mode"
            numStates: 3
            stateLabels: ["ABS", "REL", "CONST"]
        }

        // Vinyl cueing: cycles CUE(0) → CUE(1) → HOT(2) → CUE(0)
        LateNightCycleButton {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 20
            backgroundSource: LateNightTheme.lateNightButton("btn_embedded_grid.svg")
            activeBackgroundSuffix: "active"
            activeWhenNonzero: true
            activeColor: LateNightTheme.vinylCueingActiveColor
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            inactiveLabelColor: LateNightTheme.textColorMuted
            activeLabelColor: LateNightTheme.deckActiveButtonTextColor
            activeOpacity: 0.95
            inactiveOpacity: 0.72
            labelPixelSize: 11
            useBorderImageBackground: true
            backgroundBorderTop: 2
            backgroundBorderBottom: 2
            backgroundBorderLeft: 1
            backgroundBorderRight: 2
            group: root.group
            key: "vinylcontrol_cueing"
            numStates: 3
            stateLabels: ["CUE", "CUE", "HOT"]
        }

        // Passthrough toggle
        Item {
            Layout.preferredWidth: 35
            Layout.preferredHeight: 20

            property bool rejectedPress: false

            Timer {
                id: passthroughPowerWindowTimer
                interval: 300
                repeat: false
            }

            LateNightIconButton {
                anchors.fill: parent
                backgroundSource: LateNightTheme.lateNightButton("btn_embedded_grid.svg")
                activeBackgroundSuffix: "active"
                activeState: passthroughControl.value > 0
                inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                activeColor: LateNightTheme.passthroughActiveColor
                label: "PASS"
                labelPixelSize: 11
                labelColor: activeState ? LateNightTheme.deckActiveButtonTextColor : LateNightTheme.textColorMuted
                contentOpacity: activeState ? 0.95 : 0.72
                useBorderImageBackground: true
                backgroundBorderTop: 2
                backgroundBorderBottom: 2
                backgroundBorderLeft: 1
                backgroundBorderRight: 2
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                preventStealing: true

                onPressed: {
                    const targetValue = passthroughControl.value > 0 ? 0 : 1;
                    passthroughControl.parameter = targetValue;
                    parent.rejectedPress = targetValue > 0 && passthroughControl.value <= 0;
                    passthroughPowerWindowTimer.restart();
                }

                onReleased: {
                    if (parent.rejectedPress) {
                        parent.rejectedPress = false;
                        passthroughPowerWindowTimer.stop();
                        return;
                    }
                    if (!passthroughPowerWindowTimer.running) {
                        passthroughControl.parameter = passthroughControl.value > 0 ? 0 : 1;
                    }
                    passthroughPowerWindowTimer.stop();
                }

                onCanceled: {
                    parent.rejectedPress = false;
                    passthroughPowerWindowTimer.stop();
                }
            }
        }
    }
}
