import QtQuick 2.5

Item {
    id: cell
    property int slotId:0
    property int deckId: 0
    property int cellId: 0

    readonly property bool isEmpty: propState.description == "Empty"
    readonly property color color: isEmpty ? colors.colorDeckBrightGrey : colors.palette(computeBrightness(propState.description, propDisplayState.description), propColorId.value)
    readonly property color brightColor: isEmpty ? colors.colorDeckBrightGrey : colors.palette(1., propColorId.value)
    readonly property color midColor: isEmpty ? colors.colorDeckGrey : colors.palette(0.5, propColorId.value)
    readonly property color dimmedColor: isEmpty ? colors.colorDeckDarkGrey : colors.palette(0., propColorId.value)

    readonly property string name: propName.value
    readonly property bool isLooped: propPlayMode.description == "Looped"

  // AppProperty { id: propColorId;          path: "app.traktor.decks." + deckId + ".remix.cell.columns." + slotId + ".rows." + cellId + ".color_id" }
    QtObject {
        id: propColorId
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propName;             path: "app.traktor.decks." + deckId + ".remix.cell.columns." + slotId + ".rows." + cellId + ".name" }
    QtObject {
        id: propName
        property string description: "Description"
        property var value: 0
    }
  //PlayMode can be "Looped" or "OneShot"
  // AppProperty { id: propPlayMode;         path: "app.traktor.decks." + deckId + ".remix.cell.columns." + slotId + ".rows." + cellId + ".play_mode" }
    QtObject {
        id: propPlayMode
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propState;            path: "app.traktor.decks." + deckId + ".remix.cell.columns." + slotId + ".rows." + cellId + ".state" }
    QtObject {
        id: propState
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: propDisplayState;     path: "app.traktor.decks." + deckId + ".remix.cell.columns." + slotId + ".rows." + cellId + ".animation.display_state"}
    QtObject {
        id: propDisplayState
        property string description: "Description"
        property var value: 0
    }

    function computeBrightness(state, displayState) {
        if (state == "Playing" && displayState == "BrightColor" ) {return 1.;}
        return 0.5;
    }
}
