import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    property color arcColor: indicatorColor === "red" ? LateNightTheme.mixerArcMainBalanceColor : LateNightTheme.mixerArcGainLowColor
    property int arcStart: key === "gain" || key === "headGain" ? LateNightControls.Knob.ArcStart.Minimum : LateNightControls.Knob.ArcStart.Center
    required property string group
    property string indicatorColor: "orange"
    required property string key
    required property string label

    implicitHeight: 47
    implicitWidth: 42

    LateNightControls.Knob {
        anchors.horizontalCenter: parent.horizontalCenter
        backgroundSource: LateNightTheme.assetMainKnobBackground
        displayArc: true
        displayArcColor: root.arcColor
        displayArcRadius: LateNightTheme.mixerArcRadiusCompact
        displayArcStart: root.arcStart
        displayArcWidth: LateNightTheme.mixerArcWidth
        group: root.group
        height: 30
        indicatorColor: root.indicatorColor
        indicatorKind: "main"
        key: root.key
        width: 35
        y: 0
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: LateNightTheme.mixerDimTextColor
        font.bold: true
        font.pixelSize: 13
        text: root.label
    }
}
