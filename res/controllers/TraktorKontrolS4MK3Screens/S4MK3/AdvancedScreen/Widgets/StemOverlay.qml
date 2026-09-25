import QtQuick 2.15
import QtQuick.Layouts 1.1
import '../Views'

import '../Defines' as Defines

//----------------------------------------------------------------------------------------------------------------------
//  Remix Deck Overlay - for sample volume and filter value editing
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: display

    required property var deckInfo

    Dimensions {id: dimensions}
    Defines.Colors { id: colors }
    Defines.Durations { id: durations }
    Defines.Settings {id: settings}

  // MODEL PROPERTIES //
    property string showHideState: "hide"
    property int bottomMargin: 0
    property int yPositionWhenHidden: 240
    property int yPositionWhenShown: (195 - bottomMargin)

    readonly property string name: display.deckInfo.stemSelectedName

    state: display.deckInfo.stemSelected ? "show" : "hide"
    height: 40
    anchors.left: parent.left
    anchors.right: parent.right

  // dark grey background
    Rectangle {
        id: bottomInfoDetailsPanelDarkBg
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: display.height
        color: colors.colorFxHeaderBg
    // light grey background
        Rectangle {
            id:bottomInfoDetailsPanelLightBg
            anchors {
                top: parent.top
                left: parent.left
            }
            height: display.height
            width: 105
            color: colors.colorFxHeaderLightBg
        }
    }

//  // dividers
    Rectangle {
        id: fxInfoDivider0
        width:1;
        height:63;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 105
    }

    Rectangle {
        id: fxInfoDivider2
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 195
        height: display.height
    }

  // Info Details
    Rectangle {
        id: bottomInfoDetailsPanel

        height: parent.height
        clip: true
        width: parent.width
        color: "transparent"

        anchors.left: parent.left
        anchors.leftMargin: 1

        Row {
            Item {
                id: stemInfoDetailsPanel

                height: display.height
                width: 110

          // name
                Text {
                    id: stemInfoName
                    font.capitalization: Font.AllUppercase
                    text: "NAME"
                    color: settings.accentColor
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 2
                    font.pixelSize: fonts.scale(13.5)
                    anchors.leftMargin: 4
                    elide: Text.ElideRight
                }

          // value
                Text {
                    id: nameValue
                    font.capitalization: Font.AllUppercase
                    text: name
                    color: display.deckInfo.stemSelectedMidColor
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 1
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: fonts.scale(18)
                    anchors.leftMargin: 4
                    elide: Text.ElideRight
                }
            }

            Item {
                id: volumeInfoDetailsPanel

                height: display.height
                width: 85

          // volume
                Text {
                    id: volumeInfoName
                    font.capitalization: Font.AllUppercase
                    text: "VOLUME"
                    color: !display.deckInfo.stemSelectedMuted ? settings.accentColor : "grey"
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 2
                    font.pixelSize: fonts.scale(13.5)
                    anchors.leftMargin: 4
                    elide: Text.ElideRight
                }

          // value
                ProgressBar {
                    id: volume
                    progressBarHeight: 9
                    progressBarWidth: 76
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3

                    anchors.leftMargin: 5
                    anchors.rightMargin: 20

                    value: display.deckInfo.stemSelectedVolume

                    drawAsEnabled: true
                    progressBarColorIndicatorLevel: display.deckInfo.stemSelectedMuted ? "grey" : settings.accentColor
                    progressBarBackgroundColor: "black"
                }
            }

            Item {
                id: fxInfoDetailsPanel

                height: display.height
                width: 125

          // fx name
                Text {
                    id: fxInfoSampleName

                    font.capitalization: Font.AllUppercase
                    text: display.deckInfo.stemSelectedQuickFXName
                    color: display.deckInfo.stemSelectedQuickFXOn ? settings.accentColor : "grey"
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 2
                    font.pixelSize: fonts.scale(13.5)
                    anchors.leftMargin: 4
                    elide: Text.ElideRight
                }

          // value
                ProgressBar {
                    id: quickfx
                    progressBarHeight: 9
                    progressBarWidth: 115
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 20
                    anchors.bottomMargin: 3
                    anchors.leftMargin: 5

                    value: display.deckInfo.stemSelectedQuickFXValue
                    visible: fxInfoSampleName.text !== "---"

                    drawAsEnabled: true
                    progressBarColorIndicatorLevel: display.deckInfo.stemSelectedQuickFXOn ? settings.accentColor : "grey"
                    progressBarBackgroundColor: "black"
                }
            }

            // StemInfoDetails {
            //     id: bottomInfoDetails3
            //     finalValue: (type == 0 ? "Cue" : type == 1 ? "Fade-In" : type == 2 ? "Fade-Out" : type == 3 ? "Load" : type == 4 ? "Grid" : type == 5 ? "Loop" : "-")
            //     finalLabel: "TYPE"
            //     width: 50
            // }
        }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: display.bottom
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

    Behavior on y { PropertyAnimation { duration: durations.mainTransitionSpeed; easing.type: Easing.InOutQuad } }

    states: [
        State {
            name: "show";
            PropertyChanges { target: display; y: yPositionWhenShown}
        },
        State {
            name: "hide";
            PropertyChanges { target: display; y: yPositionWhenHidden}
        }
    ]
}
