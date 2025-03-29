import QtQuick 2.5
import '../Defines' as Defines
import '../Defines' as Defines

//here we assume that `colors` and `dimensions` already exists in the object hierarchy
Item {
    id: widget

    property string title: ''
    property string artist: ''
    property color backgroundColor: colors.defaultBackground
    height: dimensions.firstRowHeight
    property int radius: 		 dimensions.cornerRadius

    Defines.Settings {id: settings}
    Defines.Colors {id: colors}

    required property var deckInfo

    property int deckA: deckInfo.deckAColor
    property int deckB: deckInfo.deckBColor
    property int deckC: deckInfo.deckCColor
    property int deckD: deckInfo.deckDColor

    function colorForDeck(deckId,deckA,deckB,deckC,deckD) {
        switch (deckId) {
            case 1: return colorForDeckSingle(deckA);
            case 2: return colorForDeckSingle(deckB);
            case 3: return colorForDeckSingle(deckC);
            default: return colorForDeckSingle(deckD);
        }
    }

    function colorForDeckSingle(deck) {
        switch (deck) {
            case 0: return colors.red;
            case 1: return colors.darkOrange;
            case 2: return colors.lightOrange;
            case 3: return colors.warmYellow;
            case 4: return colors.yellow;
            case 5: return colors.lime;
            case 6: return colors.green;
            case 7: return colors.mint;
            case 8: return colors.cyan;
            case 9: return colors.turquoise;
            case 10: return colors.blue;
            case 11: return colors.plum;
            case 12: return colors.violet;
            case 13: return colors.purple;
            case 14: return colors.magenta;
            case 15: return colors.fuchsia;
            default: return colors.white;
        }
    }

    Rectangle {
        id: headerBg
        color: colorForDeck(deckInfo.deckId,deckA,deckB,deckC,deckD)
        anchors.fill: parent
        radius: widget.radius

        Text {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 2
            anchors.topMargin:	2
            font.family: "Roboto"
            font.weight: Font.Normal
            font.pixelSize: 20
            color: "black"
            text: widget.title
            elide: Text.ElideRight
            visible: deckInfo.shift ? false : true
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 2
            anchors.topMargin:	2
            font.family: "Roboto"
            font.weight: Font.Normal
            font.pixelSize: 20
            color: "black"
            text: widget.artist
            elide: Text.ElideRight
            visible: deckInfo.shift ? true : false
        }
    }
}
