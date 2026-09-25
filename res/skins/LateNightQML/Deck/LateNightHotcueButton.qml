import QtQuick
import "../../../qml" as Shared
import "../LateNightTheme"

LateNightIconButton {
    id: root

    required property string group
    required property int hotcueNumber

    activeBackgroundSuffix: "set"
    activeColor: hotcueBehavior.hotcueColor
    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    activeState: hotcueBehavior.isSet
    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    contentOpacity: 1.0
    iconSource: LateNightTheme.lateNightButton("btn__" + root.hotcueNumber + ".svg")
    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
    pressedActivatesFill: true
    pressedBackgroundSuffix: "active"
    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    pressedState: hotcueBehavior.pressed

    Shared.HotcueButtonBehavior {
        id: hotcueBehavior

        fallbackColor: LateNightTheme.accentColor
        group: root.group
        hotcueNumber: root.hotcueNumber

        onCleared: hotcuePopup.close()
        onPopupRequested: function(mouseX, mouseY) {
            hotcuePopup.x = mouseX;
            hotcuePopup.y = mouseY;
            hotcuePopup.open();
        }
    }

    LateNightHotcuePopup {
        id: hotcuePopup

        group: root.group
        hotcueNumber: root.hotcueNumber
    }
}
