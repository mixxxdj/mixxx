import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

//--------------------------------------------------------------------------------------------------------------------
//  FX CONTROLS
//--------------------------------------------------------------------------------------------------------------------

// The FxControls are located on the top of the screen and blend in if one of the top knobs is touched/changed

Item {
    id: bottomLabels

    property string showHideState: "hide"
    property int bottomMargin: 0
    property int yPositionWhenHidden: 240
    property int yPositionWhenShown: (195 - bottomMargin)
    property int hotcue: 0
    property int type: 0
    property string name: ""
    readonly property color barBgColor: "black"

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }

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
        height: bottomLabels.height
        color: colors.colorFxHeaderBg
    // light grey background
        Rectangle {
            id:bottomInfoDetailsPanelLightBg
            anchors {
                top: parent.top
                left: parent.left
            }
            height: bottomLabels.height
            width: 18
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
        anchors.leftMargin: 18
    }

    Rectangle {
        id: fxInfoDivider2
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 240
        height: 63
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
            CueInfoDetails {
                id: bottomInfoDetails1
                finalValue: hotcue
                finalLabel: "#"
                width: 18
            }

            CueInfoDetails {
                id: bottomInfoDetails2
                finalValue: name
                finalLabel: "NAME"
                width: 222
            }

            CueInfoDetails {
                id: bottomInfoDetails3
                finalValue: (type == 0 ? "Cue" : type == 1 ? "Fade-In" : type == 2 ? "Fade-Out" : type == 3 ? "Load" : type == 4 ? "Grid" : type == 5 ? "Loop" : "-")
                finalLabel: "TYPE"
                width: 50
            }
        }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: bottomLabels.bottom
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

    Item {
        id: showHide
        state: showHideState
        states: [
            State {
                name: "show";
                PropertyChanges { target: bottomLabels; y: yPositionWhenShown}
            },
            State {
                name: "hide";
                PropertyChanges { target: bottomLabels; y: yPositionWhenHidden}
            }
        ]
    }
}
