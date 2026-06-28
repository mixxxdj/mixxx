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
        id: vinylEnabledProxy
        group: root.group
        key: "vinylcontrol_enabled"
    }

    Mixxx.ControlProxy {
        id: vinylStatusProxy
        group: root.group
        key: "vinylcontrol_status"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Vinyl enable/disable toggle with status display.
        // Left-click toggles vinylcontrol_enabled.
        // Display shows vinylcontrol_status (0=off, 1=ok, 2=speed, 3=both ok).
        Item {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 20

            Image {
                anchors.fill: parent
                source: LateNightTheme.legacyTopRegionButton("medium")
                fillMode: Image.Stretch
                opacity: vinylEnabledProxy.value > 0 ? 0.95 : 0.72
            }

            Image {
                anchors.centerIn: parent
                width: Math.min(parent.width, sourceSize.width > 0 ? sourceSize.width / 2 : parent.width)
                height: Math.min(parent.height, sourceSize.height > 0 ? sourceSize.height / 2 : parent.height)
                source: {
                    var status = Math.round(vinylStatusProxy.value);
                    switch (status) {
                    case 1:
                        return LateNightTheme.assetDeckVinylControl1;
                    case 2:
                        return LateNightTheme.assetDeckVinylControl2;
                    case 3:
                        return LateNightTheme.assetDeckVinylControl3;
                    default:
                        return LateNightTheme.assetDeckVinylControl0;
                    }
                }
                fillMode: Image.PreserveAspectFit
                opacity: vinylEnabledProxy.value > 0 ? 0.95 : 0.72
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    vinylEnabledProxy.value = !vinylEnabledProxy.value;
                }
            }
        }

        // Vinyl mode: cycles ABS(0) → REL(1) → CONST(2) → ABS(0)
        LateNightCycleButton {
            Layout.preferredWidth: 46
            Layout.preferredHeight: 20
            backgroundSource: LateNightTheme.legacyTopRegionButton("medium")
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            group: root.group
            key: "vinylcontrol_mode"
            numStates: 3
            stateLabels: ["ABS", "REL", "CONST"]
        }

        // Vinyl cueing: cycles CUE(0) → CUE(1) → HOT(2) → CUE(0)
        LateNightCycleButton {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 20
            backgroundSource: LateNightTheme.legacyTopRegionButton("medium")
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            group: root.group
            key: "vinylcontrol_cueing"
            numStates: 3
            stateLabels: ["CUE", "CUE", "HOT"]
        }

        // Passthrough toggle
        LateNightControlButton {
            Layout.preferredWidth: 35
            Layout.preferredHeight: 20
            backgroundSource: LateNightTheme.legacyTopRegionButton("medium")
            label: "PASS"
            labelPixelSize: 9
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            group: root.group
            key: "passthrough"
            toggleable: true
            activeOpacity: 0.95
            inactiveOpacity: 0.72
            activeOverlayColor: LateNightTheme.accentColor
        }
    }
}
