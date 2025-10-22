import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    property string label: ""
    property string label2: ""
    property bool header: false
    property int deckId: 1

  // AppProperty { id: enabled; path: "app.traktor.decks." + deckId + ".loop.is_in_active_loop" }
    QtObject {
        id: enabled
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: size; path: "app.traktor.decks." + deckId + ".loop.size" }
    QtObject {
        id: size
        property string description: "Description"
        property var value: 0
    }

    width: 0
    height: 20

    Defines.Colors { id: colors }

    // Level indicator for knobs

  // Diverse Elements
    Item {
        id: fxInfoDetailsPanel

        height: 20
        width: parent.width

    // fx name
        Text {
            id: fxInfoSampleName
            font.capitalization: Font.AllUppercase
            text: header ? label : label2
            color: header ? settings.accentColor : (enabled.value && (size.value == label) ? "lime" : "white")
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 0
            font.pixelSize: fonts.scale(18)
            anchors.leftMargin: 4
            elide: Text.ElideRight
            horizontalAlignment: header ? Text.AlignLeft : Text.AlignHCenter
        }
    }
}
