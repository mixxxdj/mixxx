import QtQuick
import "../LateNightTheme"

LateNightControlButton {
    id: root

    required property string cueType

    key: root.cueType + "_activate"
    rightClickKey: root.cueType + "_clear"
    displayKey: root.cueType + "_enabled"
    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    iconSource: LateNightTheme.lateNightButton("btn__" + root.cueType + ".svg")
    activeBackgroundSuffix: "set"
    pressedBackgroundSuffix: "active"
    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    activeColor: LateNightTheme.specialCueActiveColor
    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
    activeOpacity: 1.0
    inactiveOpacity: 1.0
}
