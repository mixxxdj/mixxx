import QtQuick 2.15
import '../Overlays' as Overlays
import '../Defines' as Defines
import '../Widgets' as Widgets

Item {
    id: display
    anchors.fill: parent
    property color deckColor: "black"
    property var deckInfo: ({})
    Dimensions {id: dimensions}

    property real infoBoxesWidth: dimensions.infoBoxesWidth
    property real firstRowHeight: dimensions.firstRowHeight

    Rectangle {
        id: background
        color: colors.defaultBackground
        anchors.fill: parent
    }

    Image {
        id: logoImage
        anchors.fill: parent

        source: engine.getSetting("idleBackground") || "../../../../../images/templates/logo_mixxx.png"
        fillMode: Image.PreserveAspectFit
    }

  // DECK HEADER //
		// Widgets.DeckHeader
		// {
		//   id: deckHeader

		//   title:  deckInfo.headerEnabled ? deckInfo.headerText : "Live Input"
		//   artist: deckInfo.headerEnabled ? deckInfo.headerTextLong : "Live Input"

		//   height: display.firstRowHeight-6
		//   width:  4*(display.infoBoxesWidth/2+1)+10

		//   anchors.left:       parent.left
    // 	  anchors.top:        parent.top
		//   anchors.topMargin:  3
    // 	  anchors.leftMargin: 4

		// }
}
