import QtQuick 2.15

import '../Widgets' as Widgets

Item {
    id: view

    property int deckId: 0
    property int sampleWidth: 0
    property var    waveformPosition

    readonly property var colorIds: [stemColorId_1.value, stemColorId_2.value, stemColorId_3.value, stemColorId_4.value]

  //--------------------------------------------------------------------------------------------------------------------

  // AppProperty { id: stemColorId_1; path: "app.traktor.decks." + deckId + ".stems.1.color_id" }
    QtObject {
        id: stemColorId_1
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: stemColorId_2; path: "app.traktor.decks." + deckId + ".stems.2.color_id" }
    QtObject {
        id: stemColorId_2
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: stemColorId_3; path: "app.traktor.decks." + deckId + ".stems.3.color_id" }
    QtObject {
        id: stemColorId_3
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: stemColorId_4; path: "app.traktor.decks." + deckId + ".stems.4.color_id" }
    QtObject {
        id: stemColorId_4
        property string description: "Description"
        property var value: 0
    }

  //--------------------------------------------------------------------------------------------------------------------

  // Repeater {
  //   model: 4

  //   SingleWaveform {
  //     y:      index * view.height/4 + (index > 1 ? 2 : 1)
  //     width:  view.width
  //     height: 31
  //     clip:   true

  //     deckId:           view.deckId
  //     streamId:         index + 1
  //     sampleWidth:      view.sampleWidth
  //     waveformPosition: view.waveformPosition
  //     waveformColors:   colors.getWaveformColors(colorIds[index])
  //   }
  // }
}
