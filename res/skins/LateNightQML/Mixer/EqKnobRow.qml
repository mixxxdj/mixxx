import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    readonly property string effectButtonKey: "button_" + effectParameterKey
    readonly property string effectGroup: "[EqualizerRack1_" + group + "_Effect1]"
    readonly property string effectParameterKey: root.key === "filterLow" ? "parameter1" : (root.key === "filterMid" ? "parameter2" : "parameter3")
    required property string group
    required property string key
    required property url killIcon
    property bool mirror: false
    property bool showKill: true

    implicitHeight: 34
    implicitWidth: 58

    EqKillButton {
        id: eqKillButton

        anchors.left: root.mirror ? undefined : parent.left
        anchors.right: root.mirror ? parent.right : undefined
        anchors.verticalCenter: parent.verticalCenter
        group: root.effectGroup
        iconSource: root.killIcon
        key: root.effectButtonKey
        visible: root.showKill
    }
    LateNightControls.Knob {
        id: eqKnob

        anchors.left: root.mirror ? parent.left : undefined
        anchors.right: root.mirror ? undefined : parent.right
        anchors.verticalCenter: parent.verticalCenter
        backgroundSource: LateNightTheme.assetRegularKnobBackground
        displayArc: true
        displayArcColor: LateNightTheme.mixerArcEqColor
        displayArcRadius: LateNightTheme.mixerArcRadiusBig
        displayArcStart: LateNightControls.Knob.ArcStart.Center
        displayArcWidth: LateNightTheme.mixerArcWidth
        group: root.effectGroup
        height: 34
        indicatorColor: LateNightTheme.isClassic ? "white" : "grey"
        indicatorKind: "regular"
        key: root.effectParameterKey
        width: 40
    }
    Image {
        id: statusDot

        height: 9
        source: eqKillButton.value > 0 ? LateNightTheme.optionalMixerEqKillDotActiveRed : LateNightTheme.optionalMixerEqKillDotOff
        visible: LateNightTheme.isPaleMoon && !root.showKill && root.visible
        width: 9
        x: root.mirror ? eqKnob.x + 32 : eqKnob.x
        y: 27
        z: 1

        TapHandler {
            onTapped: eqKillButton.toggle()
        }
    }
}
