import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property string leftStyle: "default"
    property string rightStyle: "default"
    property bool showEqKillButtons: true
    property bool showEqKnobs: true
    property bool showXfader: true
    readonly property int volumeY: root.showEqKnobs ? 237 : 75

    implicitHeight: root.volumeY + 107 + (root.showXfader ? 15 : 0)
    implicitWidth: 62

    Item {
        anchors.right: parent.right
        height: parent.height
        width: 62

        LateNightControls.Knob {
            backgroundSource: LateNightTheme.assetRegularKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.mixerArcGainColor
            displayArcRadius: LateNightTheme.mixerArcRadiusBig
            displayArcStart: LateNightControls.Knob.ArcStart.Center
            displayArcWidth: LateNightTheme.mixerArcWidth
            group: root.group
            height: 34
            indicatorColor: "orange"
            indicatorKind: "regular"
            key: "pregain"
            width: 40
            x: 18
            y: 4
        }
        EqKnobRow {
            group: root.group
            key: "filterHigh"
            killIcon: LateNightTheme.assetMixerEqKillHighIcon
            showKill: root.showEqKillButtons
            visible: root.showEqKnobs
            width: 58
            x: 0
            y: 43
        }
        EqKnobRow {
            group: root.group
            key: "filterMid"
            killIcon: LateNightTheme.assetMixerEqKillMidIcon
            showKill: root.showEqKillButtons
            visible: root.showEqKnobs
            width: 58
            x: 0
            y: 80
        }
        EqKnobRow {
            group: root.group
            key: "filterLow"
            killIcon: LateNightTheme.assetMixerEqKillLowIcon
            showKill: root.showEqKillButtons
            visible: root.showEqKnobs
            width: 58
            x: 0
            y: 117
        }
        QuickEffectRow {
            group: root.group
            showEqKillButtons: root.showEqKillButtons
            visible: root.showEqKnobs
            width: 58
            x: 0
            y: 154
        }
        QuickEffectSelector {
            group: root.group
            visible: root.showEqKnobs
            width: 62
            x: 0
            y: 188
        }
        PflButton {
            group: root.group
            x: 28
            y: root.showEqKnobs ? 210 : 49
        }
        LateNightControls.ImageVuMeter {
            group: root.group
            height: 96
            width: 20
            x: 0
            y: root.volumeY
        }
        LateNightControls.Fader {
            backgroundSource: LateNightTheme.assetMixerVolumeSliderBackground
            group: root.group
            handleHeight: 19
            handleSource: LateNightTheme.assetMixerVolumeSliderHandle
            handleWidth: 42
            height: 107
            key: "volume"
            orientation: Qt.Vertical
            width: 42
            x: 20
            y: root.volumeY
        }
        CrossfaderAssignButton {
            group: root.group
            leftStyle: root.leftStyle
            rightStyle: root.rightStyle
            visible: root.showXfader
            x: 14
            y: root.volumeY + 107
        }
    }
}
