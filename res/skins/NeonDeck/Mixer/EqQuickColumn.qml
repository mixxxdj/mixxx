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
        killIconName: "ban"
        mirror: root.alignRight
        showKill: root.showEqKillButtons
        visible: root.showEqKnobs
    }
    EqKnobRow {
        group: root.group
        key: "filterMid"
        killIconName: "ban"
        mirror: root.alignRight
        showKill: root.showEqKillButtons
        visible: root.showEqKnobs
    }
    EqKnobRow {
        group: root.group
        key: "filterLow"
        killIconName: "ban"
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
