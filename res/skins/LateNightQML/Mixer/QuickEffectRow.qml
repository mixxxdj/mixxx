import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool mirror: false
    readonly property string quickEffectGroup: "[QuickEffectRack1_" + group + "]"
    property bool showEqKillButtons: true

    implicitHeight: 34
    implicitWidth: 58

    QuickEffectEnableButton {
        id: enableButton

        anchors.left: root.mirror ? undefined : parent.left
        anchors.right: root.mirror ? parent.right : undefined
        anchors.verticalCenter: parent.verticalCenter
        quickEffectGroup: root.quickEffectGroup
        visible: root.showEqKillButtons
    }
    Image {
        id: statusDot

        height: 9
        source: enableButton.enabledValue > 0 ? LateNightTheme.optionalMixerEqKillDotActiveGreen : LateNightTheme.optionalMixerEqKillDotOff
        visible: LateNightTheme.isPaleMoon && !root.showEqKillButtons
        width: 9
        x: root.mirror ? quickEffectKnob.x + 32 : quickEffectKnob.x
        y: 27
        z: 1

        TapHandler {
            onTapped: enableButton.toggle()
        }
    }
    LateNightControls.Knob {
        id: quickEffectKnob

        anchors.left: root.mirror ? parent.left : undefined
        anchors.right: root.mirror ? undefined : parent.right
        anchors.verticalCenter: parent.verticalCenter
        backgroundSource: LateNightTheme.assetRegularKnobBackground
        displayArc: true
        displayArcColor: LateNightTheme.mixerArcQuickEffectColor
        displayArcRadius: LateNightTheme.mixerArcRadiusBig
        displayArcStart: LateNightControls.Knob.ArcStart.Center
        displayArcWidth: LateNightTheme.mixerArcWidth
        group: root.quickEffectGroup
        height: 34
        indicatorColor: "green"
        indicatorKind: "regular"
        key: "super1"
        width: 40
    }
}
