pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../LateNightTheme"
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    readonly property int legacyBottomMargin: LateNightTheme.isClassic ? 4 : 3

    color: LateNightTheme.micAuxRackGutterColor
    implicitHeight: Math.max(micRack.implicitHeight, auxRack.implicitHeight) + legacyBottomMargin

    RowLayout {
        anchors.bottomMargin: root.legacyBottomMargin
        anchors.fill: parent
        anchors.topMargin: 0
        spacing: 0

        Controls.RackFiller {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        RowLayout {
            id: micRack

            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.leftMargin: 2
            Layout.rightMargin: 2
            spacing: 3

            MicrophoneDuckingPanel {
                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                visible: microphone1.inputConfigured || microphone2.inputConfigured || microphone3.inputConfigured || microphone4.inputConfigured
            }
            MicUnit {
                id: microphone1

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                unitNumber: 1
            }
            MicUnit {
                id: microphone2

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                unitNumber: 2
            }
            MicUnit {
                id: microphone3

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                unitNumber: 3
            }
            MicUnit {
                id: microphone4

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                unitNumber: 4
            }
        }
        Controls.RackFiller {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        RowLayout {
            id: auxRack

            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.leftMargin: 2
            Layout.rightMargin: 2
            spacing: 3

            Repeater {
                model: 4

                AuxUnit {
                    required property int index

                    Layout.alignment: Qt.AlignTop
                    Layout.fillHeight: true
                    unitNumber: index + 1
                }
            }
        }
        Controls.RackFiller {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
