import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as Controls
import "../LateNightTheme"

Rectangle {
    id: root

    readonly property bool compactDiagonal: showFourUnits && ((unit1.expanded && !unit2.expanded && !unit3.expanded && unit4.expanded) || (!unit1.expanded && unit2.expanded && unit3.expanded && !unit4.expanded))
    readonly property real firstPairHeight: Math.max(unit1.implicitHeight, unit2.implicitHeight)
    readonly property int rowSpacing: LateNightTheme.isClassic ? 4 : 3
    readonly property real secondPairHeight: showFourUnits ? Math.max(unit3.implicitHeight, unit4.implicitHeight) : 0
    readonly property real secondRowTop: rowSpacing + firstPairHeight + rowSpacing
    readonly property bool showFourUnits: showFourUnitsControl.value > 0
    readonly property real unit3Top: compactDiagonal ? unit1.y + unit1.height + rowSpacing : secondRowTop
    readonly property real unit4Top: compactDiagonal ? unit2.y + unit2.height + rowSpacing : secondRowTop

    color: LateNightTheme.effectsRackGutterColor
    implicitHeight: showFourUnits ? Math.max(unit3Top + unit3.height, unit4Top + unit4.height) + rowSpacing : rowSpacing + firstPairHeight + rowSpacing

    EffectUnit {
        id: unit1

        height: implicitHeight
        unitNumber: 1
        width: Math.round((parent.width - 4) / 2)
        x: 0
        y: root.rowSpacing
    }
    Controls.RackFiller {
        height: Math.max(0, root.firstPairHeight - unit1.height - root.rowSpacing)
        visible: !root.compactDiagonal && height > 0
        width: unit1.width - 1
        x: unit1.x
        y: unit1.y + unit1.height + root.rowSpacing
    }
    EffectUnit {
        id: unit2

        height: implicitHeight
        unitNumber: 2
        width: parent.width - x
        x: unit1.width + 4
        y: root.rowSpacing
    }
    Controls.RackFiller {
        height: Math.max(0, root.firstPairHeight - unit2.height - root.rowSpacing)
        visible: !root.compactDiagonal && height > 0
        width: unit2.width - 1
        x: unit2.x + 1
        y: unit2.y + unit2.height + root.rowSpacing
    }
    EffectUnit {
        id: unit3

        height: implicitHeight
        unitNumber: 3
        visible: root.showFourUnits
        width: Math.round((parent.width - 4) / 2)
        x: 0
        y: root.unit3Top
    }
    Controls.RackFiller {
        height: Math.max(0, root.secondPairHeight - unit3.height - root.rowSpacing)
        visible: root.showFourUnits && !root.compactDiagonal && height > 0
        width: unit3.width - 1
        x: unit3.x
        y: unit3.y + unit3.height + root.rowSpacing
    }
    EffectUnit {
        id: unit4

        height: implicitHeight
        unitNumber: 4
        visible: root.showFourUnits
        width: parent.width - x
        x: unit3.width + 4
        y: root.unit4Top
    }
    Controls.RackFiller {
        height: Math.max(0, root.secondPairHeight - unit4.height - root.rowSpacing)
        visible: root.showFourUnits && !root.compactDiagonal && height > 0
        width: unit4.width - 1
        x: unit4.x + 1
        y: unit4.y + unit4.height + root.rowSpacing
    }
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
