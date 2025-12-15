import QtQuick 2.5
import '../Widgets' as Widgets
import '../Overlays' as Overlays

//----------------------------------------------------------------------------------------------------------------------
//  Stem Screen View - UI of the screen for stems
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: display

  // MODEL PROPERTIES //
    required property var deckInfo

    width: 320
    height: 240

    TrackDeck {
        id: trackScreen
        deckInfo: display.deckInfo
        anchors.fill: parent
    }

  // STEM  OVERLAY //
    Widgets.StemOverlay {
        deckInfo: display.deckInfo
    }
}
