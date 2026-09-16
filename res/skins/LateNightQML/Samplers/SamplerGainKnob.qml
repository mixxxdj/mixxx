import "../Controls" as Controls
import "../LateNightTheme"
import QtQuick

Controls.Knob {
    id: root

    angle: LateNightTheme.isClassic ? 135 : 130
    backgroundSource: LateNightTheme.assetSmallKnobBackground
    displayArc: !LateNightTheme.isClassic
    displayArcColor: LateNightTheme.samplerGainArcColor
    displayArcOffsetY: 2
    displayArcRadius: 12.5
    displayArcStart: Controls.Knob.ArcStart.Minimum
    displayArcWidth: 2
    implicitHeight: 30
    implicitWidth: 35
    indicatorColor: "orange"
    indicatorKind: "small"
    key: "pregain"
}
