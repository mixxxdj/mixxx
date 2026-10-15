pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    readonly property int legacyBottomMargin: LateNightTheme.isClassic ? 4 : 3

    color: LateNightTheme.micAuxRackGutterColor
    implicitHeight: Math.max(micRack.implicitHeight, auxRack.implicitHeight) + legacyBottomMargin

    RowLayout {
        anchors.fill: parent
        anchors.bottomMargin: root.legacyBottomMargin
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
                visible: numMicrophonesControl.value > 0
            }
            Repeater {
                model: 4

                MicUnit {
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
    Mixxx.ControlProxy {
        id: numMicrophonesControl

        group: "[App]"
        key: "num_microphones"
    }
}
