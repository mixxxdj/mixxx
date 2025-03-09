import QtQuick 2.15

import './AdvancedScreen/Defines' as Defines
import './AdvancedScreen/Views' as Views
import './AdvancedScreen' as S4MK3

//----------------------------------------------------------------------------------------------------------------------
//  S4MK3 Screen - manage top/bottom deck of one screen
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: screen

    required property bool isLeftScreen

    //--------------------------------------------------------------------------------------------------------------------

    readonly property int topDeckId: isLeftScreen ? 1 : 2
    readonly property int bottomDeckId: isLeftScreen ? 3 : 4
    property bool propTopDeckFocus: true

    Defines.Font {id: fonts}
    Defines.Utils {id: utils}
    Defines.Settings {id: settings}
    Defines.Durations {id: durations}
    Defines.Colors {id: colors}

    Component.onCompleted: {
        if (typeof engine.makeSharedDataConnection === "function") {
            engine.makeSharedDataConnection(screen.onSharedDataUpdate)
            screen.onSharedDataUpdate(engine.getSharedData())
        }
    }

    function onSharedDataUpdate(data) {
        if (typeof data === "object" && typeof data.group === "object") {
            propTopDeckFocus = data.group[isLeftScreen ? 'leftdeck' : 'rightdeck'] === `[Channel${screen.topDeckId}]`
        }
    }

    width: 320
    height: 240
    clip: true

    /*
      A screen is visible if -
      The deck is in focus and the linked deck is not selecting a sample slot
      OR
      The deck is not in focus but a sample slot is selected
    */
    S4MK3.DeckScreen {
        id: topDeckScreen
        deckId: topDeckId
        visible: propTopDeckFocus
        anchors.fill: parent
    }

    S4MK3.DeckScreen {
        id: bottomDeckScreen
        deckId: bottomDeckId
        visible: !propTopDeckFocus
        anchors.fill: parent
    }
}
