import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines
import '../ViewModels' as ViewModels

//--------------------------------------------------------------------------------------------------------------------
//  FX CONTROLS
//--------------------------------------------------------------------------------------------------------------------

// The FxControls are located on the top of the screen and blend in if one of the top knobs is touched/changed

Item {
    id: view

    property string showHideState: "hide"
    property int bottomMargin: 0
    property int yPositionWhenHidden: 240
    property int yPositionWhenShown: (240 - height)
    property string name: ""
    readonly property color barBgColor: "black"
    property int deckId: 1

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }
    Defines.Settings { id: settings }

    required property var deckInfo

    height: 65
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
        height: view.height
        color: colors.colorFxHeaderBg
    }

  // dividers
    Rectangle {
        id: fxInfoDivider0
        width:1;
        height:80;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 80
    }

  // dividers
    Rectangle {
        id: fxInfoDivider1
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 160
        height: 80
    }

    Rectangle {
        id: fxInfoDivider2
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 240
        height: 80
    }

  // dividers
    Rectangle {
        id: fxInfoDivider3
        width:360;
        height:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
    }

  // dividers
    Rectangle {
        id: fxInfoDivider4
        width:360;
        height:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.left: parent.left
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

        Column {
            Row {
                LoopInfoDetails {
                    id: header
                    label: "LOOP"
                    width: 200
                    header: true
                }
            }

            Row {
                LoopInfoDetails {
                    id: bottomInfoDetails1
                    label: deckInfo.loopSizePad1
                    label2: deckInfo.loopSizePad1
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails2
                    label: deckInfo.loopSizePad2
                    label2: deckInfo.loopSizePad2
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails3
                    label: deckInfo.loopSizePad3
                    label2: deckInfo.loopSizePad3
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails4
                    label: deckInfo.loopSizePad4
                    label2: deckInfo.loopSizePad4
                    deckId: deckId
                    width: 80
                }
            }

            Row {
                LoopInfoDetails {
                    id: bottomInfoDetails5
                    label: deckInfo.loopSizePad5
                    label2: deckInfo.loopSizePad5
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails6
                    label: deckInfo.loopSizePad6
                    label2: deckInfo.loopSizePad6
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails7
                    label: deckInfo.loopSizePad7
                    label2: deckInfo.loopSizePad7
                    deckId: deckId
                    width: 80
                }
                LoopInfoDetails {
                    id: bottomInfoDetails8
                    label: deckInfo.loopSizePad8
                    label2: deckInfo.loopSizePad8
                    deckId: deckId
                    width: 80
                }
            }
        }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: view.bottom
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
                PropertyChanges { target: view; y: yPositionWhenShown}
            },
            State {
                name: "hide";
                PropertyChanges { target: view; y: yPositionWhenHidden}
            }
        ]
    }
}
