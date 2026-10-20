import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

Column {
    id: root

    property bool alignRight: false
    required property string group
    property bool showEqKillButtons: true
    property bool showEqKnobs: true

    spacing: 2

    EqKnobRow {
        group: root.group
        key: "filterHigh"
        killIcon: LateNightTheme.assetMixerEqKillHighIcon
        mirror: root.alignRight
        showKill: root.showEqKillButtons
        visible: root.showEqKnobs
    }
    EqKnobRow {
        group: root.group
        key: "filterMid"
        killIcon: LateNightTheme.assetMixerEqKillMidIcon
        mirror: root.alignRight
        showKill: root.showEqKillButtons
        visible: root.showEqKnobs
    }
    EqKnobRow {
        group: root.group
        key: "filterLow"
        killIcon: LateNightTheme.assetMixerEqKillLowIcon
        mirror: root.alignRight
        showKill: root.showEqKillButtons
        visible: root.showEqKnobs
    }
    QuickEffectRow {
        group: root.group
        mirror: root.alignRight
        showEqKillButtons: root.showEqKillButtons
        visible: root.showEqKnobs
    }
}
