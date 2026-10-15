pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

// VirtualDJ style pad bank: a grid of hotcue pads that stretches to the space
// it is given, so the pads can be shown either as a wide row above the jog or
// as a block filling the deck's control column.
GridLayout {
    id: root

    required property string group
    property int hotcueCount: 8
    property int padColumns: 4

    columnSpacing: 1
    columns: Math.max(1, root.padColumns)
    rowSpacing: 1
    rows: Math.max(1, Math.ceil(root.hotcueCount / columns))

    Repeater {
        model: root.hotcueCount

        LateNightHotcueButton {
            required property int index

            Layout.fillHeight: true
            Layout.fillWidth: true
            group: root.group
            hotcueNumber: index + 1
        }
    }
}
