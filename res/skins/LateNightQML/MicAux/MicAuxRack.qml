pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    color: LateNightTheme.micAuxRackGutterColor
    implicitHeight: 85

    RowLayout {
        anchors.fill: parent
        anchors.bottomMargin: 3
        anchors.topMargin: 3
        spacing: 0

        Controls.RackFiller {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.fillHeight: true
            Layout.leftMargin: 2
            Layout.rightMargin: 2
            spacing: 3

            MicrophoneDuckingPanel {
                Layout.alignment: Qt.AlignTop
                visible: numMicrophonesControl.value > 0
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
        Controls.RackFiller {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.fillHeight: true
            Layout.leftMargin: 2
            Layout.rightMargin: 2
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
