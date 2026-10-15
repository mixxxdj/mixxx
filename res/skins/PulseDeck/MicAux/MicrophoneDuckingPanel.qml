import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    color: LateNightTheme.micAuxPanelColor
    implicitHeight: 59
    implicitWidth: 48

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        DeckControls.LateNightIconButton {
            id: duckModeButton

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 24
            Layout.preferredWidth: 42
            activeBackgroundSuffix: "active"
            activeColor: LateNightTheme.activePlayCueColor
            activeState: duckModeBehavior.currentState > 0
            backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
            contentOpacity: 1
            iconName: {
                switch (duckModeBehavior.currentState) {
                case 1:
                    return "bolt";
                case 2:
                    return "sliders";
                default:
                    return "power-off";
                }
            }
            stretchIcon: true

            Shared.ControlCycleButtonBehavior {
                id: duckModeBehavior

                anchors.fill: parent
                group: "[Master]"
                key: "talkoverDucking"
            }
        }
        Controls.Knob {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 30
            Layout.preferredWidth: 35
            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.micAuxDuckingArcColor
            displayArcStart: Controls.Knob.ArcStart.Maximum
            group: "[Master]"
            indicatorColor: "red"
            indicatorKind: "small"
            key: "duckStrength"
        }
    }
}
