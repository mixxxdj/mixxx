import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import QtQuick

DeckControls.LateNightIconButton {
    id: root

    required property string group
    required property int hotcueNumber

    activeBackgroundSuffix: "set"
    activeColor: hotcueBehavior.hotcueColor
    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    activeState: hotcueBehavior.isSet
    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    contentOpacity: 1
    fillMargin: 1
    fillRadius: 2
    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__" + root.hotcueNumber + ".svg")
    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
    pressedActivatesFill: true
    pressedBackgroundSuffix: "active"
    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
    pressedState: hotcueBehavior.pressed
    solidFillEnabled: true

    Shared.HotcueButtonBehavior {
        id: hotcueBehavior

        fallbackColor: LateNightTheme.samplerColor
        group: root.group
        hotcueNumber: root.hotcueNumber

        onCleared: popup.close()
        onPopupRequested: function (mouseX, mouseY) {
            popup.x = mouseX;
            popup.y = mouseY;
            popup.open();
        }
    }
    Shared.HotcuePopup {
        id: popup

        hotcue: hotcue
    }
    Shared.Hotcue {
        id: hotcue

        group: root.group
        hotcueNumber: root.hotcueNumber
    }
}
