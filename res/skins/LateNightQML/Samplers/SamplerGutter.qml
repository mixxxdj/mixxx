import "../Controls" as Controls
import "../LateNightTheme"
import QtQuick

Controls.Panel {
    bottomBorderColor: LateNightTheme.samplerExpanderBottomBorderColor
    color: LateNightTheme.samplerExpanderColor
    implicitHeight: 40
    implicitWidth: 20
    leftBorderColor: LateNightTheme.samplerExpanderLeftBorderColor
    radius: LateNightTheme.isClassic ? 2 : 1
    rightBorderColor: LateNightTheme.samplerExpanderRightBorderColor
    topBorderColor: LateNightTheme.samplerExpanderTopBorderColor
}
