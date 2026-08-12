import "../Controls" as Controls
import "../LateNightTheme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
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
        Shared.ControlKnob {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 35
            Layout.preferredWidth: 35
            arcStart: MixxxControls.Knob.ArcStart.Maximum
            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
            color: LateNightTheme.activePlayCueColor
            group: "[Master]"
            key: "duckStrength"
            shadowSource: ""
        }
    }
    Mixxx.ControlProxy {
        id: duckModeControl

        group: "[Master]"
        key: "talkoverDucking"
    }
}
