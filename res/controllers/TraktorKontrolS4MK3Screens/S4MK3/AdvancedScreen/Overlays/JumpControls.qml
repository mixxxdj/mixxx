import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Widgets' as Widgets
import '../Defines' as Defines

//--------------------------------------------------------------------------------------------------------------------
//  FX CONTROLS
//--------------------------------------------------------------------------------------------------------------------

// The FxControls are located on the top of the screen and blend in if one of the top knobs is touched/changed

Item {
    id: fxLabels

    property string showHideState: "hide"
    property int bottomMargin: 0
    property int yPositionWhenHidden: 240
    property int yPositionWhenShown: (240 - height)
    property string name: ""
    readonly property color barBgColor: "black"

    required property var deckInfo
    readonly property bool shift: deckInfo.shift

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }
    Defines.Settings { id: settings }

    height: 65
    anchors.left: parent.left
    anchors.right: parent.right

    Mixxx.ControlProxy {
        id: beatjump
        group: deckInfo.group
        key: "beatjump_size"
    }

  // dark grey background
    Rectangle {
        id: bottomInfoDetailsPanelDarkBg
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: fxLabels.height
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
                JumpInfoDetails {
                    id: header
                    label: "MOVE/BEATJUMP"
                    width: 200
                    header: true
                }
            }

            Row {
                JumpInfoDetails {
                    id: bottomInfoDetails1
                    label: deckInfo.jumpSizePad1 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad1, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails2
                    label: deckInfo.jumpSizePad2 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad2, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails3
                    label: deckInfo.jumpSizePad3 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad3, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails4
                    label: deckInfo.jumpSizePad4 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad4, shift)
                    back: shift
                    width: 80
                }
            }

            Row {
                JumpInfoDetails {
                    id: bottomInfoDetails5
                    label: deckInfo.jumpSizePad5 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad5, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails6
                    label: deckInfo.jumpSizePad6 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad6, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails7
                    label: deckInfo.jumpSizePad7 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad7, shift)
                    back: shift
                    width: 80
                }
                JumpInfoDetails {
                    id: bottomInfoDetails8
                    label: deckInfo.jumpSizePad8 === "??" ? (shift ? "- " : " ") + (beatjump.value < 1 ? `1 / ${1/beatjump.value}` : `${beatjump.value}`) : getValue(deckInfo.jumpSizePad8, shift)
                    back: shift
                    width: 80
                }
            }
        }
    }

    function getValue(size, shift) {
        if (parseFloat(size)) {
            return (shift ? "- " : " ") + (size < 1 ? `1 / ${1/size}` : `${size}`)
        } else if (size === "??") {
            return null;
        } else {
            return size
        }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: fxLabels.bottom
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
                PropertyChanges { target: fxLabels; y: yPositionWhenShown}
            },
            State {
                name: "hide";
                PropertyChanges { target: fxLabels; y: yPositionWhenHidden}
            }
        ]
    }
}
