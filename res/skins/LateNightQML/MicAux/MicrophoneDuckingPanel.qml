import "../Controls" as Controls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    color: LateNightTheme.micAuxPanelColor
    implicitHeight: 78
    implicitWidth: 50

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 24
            Layout.preferredWidth: 42

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: {
                    switch (Math.round(duckModeControl.value)) {
                    case 1:
                        return LateNightTheme.assetMicDuckAutoButton;
                    case 2:
                        return LateNightTheme.assetMicDuckManualButton;
                    default:
                        return LateNightTheme.assetMicDuckOffButton;
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: duckModeControl.value = (Math.round(duckModeControl.value) + 1) % 3
            }
        }
        Controls.Knob {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 35
            Layout.preferredWidth: 35
            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.activePlayCueColor
            displayArcStart: Controls.Knob.ArcStart.Maximum
            group: "[Master]"
            indicatorColor: "orange"
            indicatorKind: "small"
            key: "duckStrength"
        }
    }
    Mixxx.ControlProxy {
        id: duckModeControl

        group: "[Master]"
        key: "talkoverDucking"
    }
}
