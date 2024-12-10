import QtQuick 2.15

import '../Defines' as Defines
import '../Widgets' as Widgets

import Mixxx 1.0 as Mixxx

//--------------------------------------------------------------------------------------------------------------------
//  FX CONTROLS
//--------------------------------------------------------------------------------------------------------------------

// The FxControls are located on the top of the screen and blend in if one of the top knobs is touched/changed

Item {
    id: topLabels

    required property var deckInfo

    property int topMargin: 0

    property int yPositionWhenHidden: -25
    property int yPositionWhenShown: topMargin

    readonly property color barBgColor: "black"

    property var fxModel: Mixxx.EffectsManager.quickChainPresetModel

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }
    Defines.Settings {id: settings}

    height: 25
    anchors.left: parent.left
    anchors.right: parent.right

  // dark grey background
    Rectangle {
        id: topInfoDetailsPanelDarkBg
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: topLabels.height
        color: colors.colorFxHeaderBg
    // light grey background
        // Rectangle {
        //     id:topInfoDetailsPanelLightBg
        //     anchors {
        //         top: parent.top
        //         left: parent.left
        //     }
        //     height: topLabels.height
        //     width: 240
        //     color: colors.colorFxHeaderLightBg
        // }
    }

  // Info Details
    Rectangle {
        id: topInfoDetailsPanel

        height: parent.height
        // clip: true
        width: parent.width
        color: "transparent"

        anchors.left: parent.left
        anchors.leftMargin: 1

        // Row {
        //     id: controlRow

            // Item {
            //   id: quickFxDetailsPanel

            //   height: display.height
            //   width: 260

          // name
        Text {
            id: stemInfoName
            font.capitalization: Font.AllUppercase
            text: "SELECTED QUICK FX"
            color: settings.accentColor

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10

            font.pixelSize: fonts.scale(13.5)
            elide: Text.ElideRight
        }

          // value
        Text {
            id: nameValue
            font.capitalization: Font.AllUppercase
            text: fxModel.get(deckInfo.quickFXSelected).display || "---"
            color: colors.colorWhite

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10

            font.pixelSize: fonts.scale(13.5)
            elide: Text.ElideRight
        }
            // }
        // }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: topLabels.bottom
        width: parent.width
        color: colors.colorBlack
        height: 2
    }
    Rectangle {
        id: headerShadow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerBlackLine.bottom
        height: 6
        gradient: Gradient {
            GradientStop { position: 1.0; color: colors.colorBlack0 }
            GradientStop { position: 0.0; color: colors.colorBlack63 }
        }
        visible: false
    }

  //------------------------------------------------------------------------------------------------------------------
  //  STATES
  //------------------------------------------------------------------------------------------------------------------

    Behavior on y { PropertyAnimation { duration: durations.overlayTransition; easing.type: Easing.InOutQuad } }

    state: deckInfo.quickFXSelected != null ? "show" : "hide"
    states: [
        State {
            name: "show";
            PropertyChanges { target: topLabels; y: yPositionWhenShown}
        },
        State {
            name: "hide";
            PropertyChanges { target: topLabels; y: -height}
        }
    ]
}
