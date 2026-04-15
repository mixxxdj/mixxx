import QtQuick 2.5

Item {
    id: hotcuesModel
    property int deckId: 0

    readonly property alias activeHotcue: activeHotcueModel
    readonly property var array:
        [
        hotcueModel1,
        hotcueModel2,
        hotcueModel3,
        hotcueModel4,
        hotcueModel5,
        hotcueModel6,
        hotcueModel7,
        hotcueModel8
    ]

    Item {
        id: activeHotcueModel
        readonly property real position: activePos.value
        readonly property real length: activeLength.value
        readonly property string type: activeType.value
        readonly property string name: activeName.value

    // AppProperty { id: activePos;      path: "app.traktor.decks." + deckId + ".track.cue.active.start_pos" }
        QtObject {
            id: activePos
            property string description: "Description"
            property var value: 0
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: activeLength;   path: "app.traktor.decks." + deckId + ".track.cue.active.length" }
        QtObject {
            id: activeLength
            property string description: "Description"
            property var value: 0
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: activeType;     path: "app.traktor.decks." + deckId + ".track.cue.active.type"   }
        QtObject {
            id: activeType
            property string description: "Description"
            property var value: 0
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: activeName;     path: "app.traktor.decks." + deckId + ".track.cue.active.name"   }
        QtObject {
            id: activeName
            property string description: "Description"
            property var value: 0
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    }

    HotCue { id: hotcueModel1; index: 0 }
    HotCue { id: hotcueModel2; index: 1 }
    HotCue { id: hotcueModel3; index: 2 }
    HotCue { id: hotcueModel4; index: 3 }
    HotCue { id: hotcueModel5; index: 4 }
    HotCue { id: hotcueModel6; index: 5 }
    HotCue { id: hotcueModel7; index: 6 }
    HotCue { id: hotcueModel8; index: 7 }
}
