import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import QtQuick

Controls.Panel {
    id: root

    bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
    color: LateNightTheme.micAuxPanelColor
    implicitHeight: LateNightTheme.isClassic ? 62 : 63
    implicitWidth: 48
    leftBorderColor: LateNightTheme.mixerPanelBorderLeft
    rightBorderColor: LateNightTheme.mixerPanelBorderRight
    topBorderColor: LateNightTheme.mixerPanelBorderTop

    DeckControls.LateNightIconButton {
        id: duckModeButton

        activeBackgroundSuffix: "active"
        activeColor: LateNightTheme.activePlayCueColor
        activeState: duckModeBehavior.currentState > 0
        backgroundSource: LateNightTheme.lateNightButton("btn_embedded_medium.svg")
        contentOpacity: 1
        fillMargin: 1
        height: 24
        iconSource: {
            switch (duckModeBehavior.currentState) {
            case 1:
                return LateNightTheme.assetMicDuckAutoButton;
            case 2:
                return LateNightTheme.assetMicDuckManualButton;
            default:
                return LateNightTheme.assetMicDuckOffButton;
            }
        }
        pressedBackgroundSuffix: "active"
        pressedState: duckMouse.pressed
        solidFillEnabled: true
        stretchIcon: false
        width: 42
        x: 3
        y: 3

        Shared.ControlCycleButtonBehavior {
            id: duckModeBehavior

            anchors.fill: parent
            group: "[Master]"
            handlePointerInput: false
            key: "talkoverDucking"
        }
        MouseArea {
            id: duckMouse

            acceptedButtons: Qt.LeftButton
            anchors.fill: parent

            onPressed: duckModeBehavior.cycle()
        }
    }
    Controls.Knob {
        anchors.horizontalCenter: parent.horizontalCenter
        backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
        displayArc: true
        displayArcColor: LateNightTheme.micAuxDuckingArcColor
        displayArcStart: Controls.Knob.ArcStart.Maximum
        group: "[Master]"
        height: 30
        indicatorColor: "red"
        indicatorKind: "small"
        key: "duckStrength"
        width: 35
        y: LateNightTheme.isClassic ? 29 : 28
    }
}
