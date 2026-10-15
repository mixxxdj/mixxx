import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as Controls
import "../LateNightTheme"

Rectangle {
    id: root

    readonly property bool compactDiagonal: showFourUnits && ((unit1.expanded && !unit2.expanded && !unit3.expanded && unit4.expanded) || (!unit1.expanded && unit2.expanded && unit3.expanded && !unit4.expanded))
    readonly property real firstPairHeight: Math.max(unit1.implicitHeight, unit2.implicitHeight)
    readonly property int rowSpacing: LateNightTheme.deckRowGutter
    readonly property real secondPairHeight: showFourUnits ? Math.max(unit3.implicitHeight, unit4.implicitHeight) : 0
    readonly property real secondRowTop: firstPairHeight + rowSpacing
    readonly property bool showFourUnits: showFourUnitsControl.value > 0
    // VirtualDJ style: the effect panels sit against the outer edges with an
    // empty middle instead of each unit stretching across half the window.
    readonly property real unitWidth: Math.min(420, Math.round(root.width * 0.32))
    property real leftUnitEnd: root.unitWidth
    property real rightUnitStart: root.width - root.unitWidth
    readonly property real unit3Top: compactDiagonal ? unit1.y + unit1.height + rowSpacing : secondRowTop
    readonly property real unit4Top: compactDiagonal ? unit2.y + unit2.height + rowSpacing : secondRowTop
    // VirtualDJ style column: the units of one side stack vertically, so the
    // rack can live at the outer edge of the deck row instead of in a band of
    // its own. Units 1 and 3 go to the left column, 2 and 4 to the right one.
    property bool vertical: false
    property string verticalSide: "left"
    readonly property real verticalUnitHeight: 154
    readonly property real verticalHeight: (showFourUnits ? verticalUnitHeight * 2 + rowSpacing : verticalUnitHeight) + rowSpacing
    readonly property bool verticalLeft: root.vertical && root.verticalSide === "left"
    readonly property bool verticalRight: root.vertical && root.verticalSide === "right"

    color: LateNightTheme.effectsRackGutterColor
    implicitHeight: root.vertical
            ? root.verticalHeight
            : (showFourUnits ? Math.max(unit3Top + unit3.height, unit4Top + unit4.height) + rowSpacing : firstPairHeight + rowSpacing)

    EffectUnit {
        id: unit1

        forceExpanded: root.vertical
        height: root.vertical ? root.verticalUnitHeight : implicitHeight
        unitNumber: 1
        visible: !root.vertical || root.verticalLeft
        width: root.vertical ? root.width : Math.max(0, root.leftUnitEnd - unit1.x)
        x: 0
        y: 0
    }
    Controls.RackFiller {
        height: Math.max(0, root.firstPairHeight - unit1.height - root.rowSpacing)
        visible: !root.vertical && !root.compactDiagonal && height > 0
        width: Math.max(0, root.leftUnitEnd - x)
        x: unit1.x
        y: unit1.y + unit1.height + root.rowSpacing
    }
    EffectUnit {
        id: unit2

        forceExpanded: root.vertical
        height: root.vertical ? root.verticalUnitHeight : implicitHeight
        unitNumber: 2
        visible: !root.vertical || root.verticalRight
        width: root.vertical ? root.width : parent.width - x
        x: root.vertical ? 0 : root.rightUnitStart
        y: 0
    }
    Controls.RackFiller {
        height: Math.max(0, root.firstPairHeight - unit2.height - root.rowSpacing)
        visible: !root.vertical && !root.compactDiagonal && height > 0
        width: parent.width - x
        x: root.rightUnitStart
        y: unit2.y + unit2.height + root.rowSpacing
    }
    EffectUnit {
        id: unit3

        forceExpanded: root.vertical
        height: root.vertical ? root.verticalUnitHeight : implicitHeight
        unitNumber: 3
        visible: root.showFourUnits && (!root.vertical || root.verticalLeft)
        width: root.vertical ? root.width : Math.max(0, root.leftUnitEnd - unit3.x)
        x: 0
        y: root.vertical ? unit1.height + root.rowSpacing : root.unit3Top
    }
    Controls.RackFiller {
        height: Math.max(0, root.secondPairHeight - unit3.height - root.rowSpacing)
        visible: root.showFourUnits && !root.vertical && !root.compactDiagonal && height > 0
        width: Math.max(0, root.leftUnitEnd - x)
        x: unit3.x
        y: unit3.y + unit3.height + root.rowSpacing
    }
    EffectUnit {
        id: unit4

        forceExpanded: root.vertical
        height: root.vertical ? root.verticalUnitHeight : implicitHeight
        unitNumber: 4
        visible: root.showFourUnits && (!root.vertical || root.verticalRight)
        width: root.vertical ? root.width : parent.width - x
        x: root.vertical ? 0 : root.rightUnitStart
        y: root.vertical ? unit2.height + root.rowSpacing : root.unit4Top
    }
    Controls.RackFiller {
        height: Math.max(0, root.secondPairHeight - unit4.height - root.rowSpacing)
        visible: root.showFourUnits && !root.compactDiagonal && height > 0
        width: parent.width - x
        x: root.rightUnitStart
        y: unit4.y + unit4.height + root.rowSpacing
    }
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
