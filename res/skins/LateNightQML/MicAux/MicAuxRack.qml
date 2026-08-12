pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    implicitHeight: 82

    RowLayout {
        anchors.fill: parent
        spacing: 3

        Controls.Panel {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LateNightTheme.micAuxPanelColor
        }
        RowLayout {
            Layout.fillHeight: true
            spacing: 3

            MicrophoneDuckingPanel {
                Layout.alignment: Qt.AlignTop
                visible: showDuckingControls.value > 0
            }
            Repeater {
                model: 4

                MicUnit {
                    required property int index

                    Layout.alignment: Qt.AlignTop
                    unitNumber: index + 1
                }
            }
        }
        Controls.Panel {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LateNightTheme.micAuxPanelColor
        }
        RowLayout {
            Layout.fillHeight: true
            spacing: 3

            Repeater {
                model: 4

                AuxUnit {
                    required property int index

                    Layout.alignment: Qt.AlignTop
                    unitNumber: index + 1
                }
            }
        }
        Controls.Panel {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LateNightTheme.micAuxPanelColor
        }
    }
    Mixxx.ControlProxy {
        id: showDuckingControls

        group: "[Skin]"
        key: "show_ducking_controls"
    }
}
