import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Defines' as Defines
import '../Defines' as Defines
import '../ViewModels' as ViewModels

Item {
    id: view

    property int deckId: 1

    Defines.Colors {id: colors}
    Defines.Settings {id: settings}

    required property var deckInfo

    readonly property int stemCount: deckInfo.stemCount
    readonly property var stemColors: ["green", "blue", "red", settings.accentColor]

    property var indicatorHeight: [31 , 31 , 31 , 31]

  //--------------------------------------------------------------------------------------------------------------------
  // There is one pixel space between the color-indicator-rectangles. In this space, you can see the beatgrid/cuePoints,
  // which is not what we want. Therefore I added this Rectalgles in the same color as the background. This rectangles hide
  // the beatgrid/cuePoints.
    Rectangle { x: 0; y: 0; width: 5; height: view.height; color: colors.colorBlack75 }
    Rectangle { x: view.width - width; y: 0; width: 5; height: view.height; color: colors.colorBlack75 }

  //--------------------------------------------------------------------------------------------------------------------

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(`[Channel${deckId}]`)
    readonly property var currentPlayer: deckPlayer.currentPlayer

    function indicatorY(index) {
        var y = 0;
        for (var i=0; i<index; ++i) {
            y = y + indicatorHeight[i] + 1;
        }
        return y;
    }

    Repeater {
        model: stemCount

        Item {
            id: indicator

            required property int index

            property color stemColor: view.currentPlayer?.stemsModel.get(index).color || "black"

            width: view.width; x: 0
            height: indicatorHeight[index]; y: indicatorY(index)

            Rectangle {
                width:1
                height: indicatorHeight[index]
                color: "black"
            }
            Mixxx.ControlProxy {
                group: `[Channel${view.deckId}_Stem${indicator.index+1}]`
                key: "volume"
                id: stemVolume
            }
            Mixxx.ControlProxy {
                group: `[Channel${view.deckId}_Stem${indicator.index+1}]`
                key: "mute"
                id: stemMute
            }
            Rectangle {
                id: colorRect2
                x: view.width - width;
                width: deckInfo.stemSelected && deckInfo.stemSelectedIdx == index ? 10 : 5
                height: indicatorHeight[index]*stemVolume.value
                color: stemColor
                opacity: stemMute.value ? 0.4 : 1.0
            }
        }
    }
}
