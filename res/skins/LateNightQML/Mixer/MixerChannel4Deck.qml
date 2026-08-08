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

    implicitHeight: 405
    implicitWidth: root.showEqKillButtons ? 62 : 44

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
            y: 210
        }
        LateNightControls.ImageVuMeter {
            group: root.group
            height: 96
            width: 20
            x: 0
            y: 237
        }
        LateNightControls.Fader {
            backgroundSource: LateNightTheme.assetMixerVolumeSliderBackground
            group: root.group
            handleSource: LateNightTheme.assetMixerVolumeSliderHandle
            height: 107
            key: "volume"
            orientation: Qt.Vertical
            width: 42
            x: 20
            y: 237
        }
        CrossfaderAssignButton {
            activeStyle: root.leftStyle
            group: root.group
            label: "left"
            side: 0
            x: 14
            y: 344
        }
        CrossfaderAssignButton {
            activeStyle: "warning"
            group: root.group
            label: "mid"
            side: 1
            x: 25
            y: 344
        }
        CrossfaderAssignButton {
            activeStyle: root.rightStyle
            group: root.group
            label: "right"
            side: 2
            x: 36
            y: 344
        }
    }
}
