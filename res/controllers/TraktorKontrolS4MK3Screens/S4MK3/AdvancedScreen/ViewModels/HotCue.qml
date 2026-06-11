import QtQuick 2.5

Item {
    id: hotcue
    readonly property real position: propPosition.value
    readonly property real length: propLength.value
    readonly property string type: propType.value
    readonly property string name: propName.value
    readonly property bool exists: propExists.value
    property int index: 0

  // AppProperty { id: propPosition;   path: "app.traktor.decks." + deckId + ".track.cue.hotcues." + (index + 1) + ".start_pos" }
    QtObject {
        id: propPosition
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propLength;     path: "app.traktor.decks." + deckId + ".track.cue.hotcues." + (index + 1) + ".length"    }
    QtObject {
        id: propLength
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propType;       path: "app.traktor.decks." + deckId + ".track.cue.hotcues." + (index + 1) + ".type"   }
    QtObject {
        id: propType
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propName;       path: "app.traktor.decks." + deckId + ".track.cue.hotcues." + (index + 1) + ".name"   }
    QtObject {
        id: propName
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propExists;     path: "app.traktor.decks." + deckId + ".track.cue.hotcues." + (index + 1) + ".exists" }
    QtObject {
        id: propExists
        property string description: "Description"
        property var value: 0
    }
}
