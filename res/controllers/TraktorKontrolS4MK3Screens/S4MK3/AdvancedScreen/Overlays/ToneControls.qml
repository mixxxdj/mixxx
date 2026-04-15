import QtQuick 2.15

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
    property int deckId: 1
    property real adjustVal: 0.00
    property string adjust: adjustVal.toFixed(0)

    Timer {
        id: toneTimer
        property bool blink: false

        interval: 250
        repeat: true
        running: adjust != 0

        onTriggered: {
            blink = !blink;
        }

        onRunningChanged: {
            blink = running;
        }
    }

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }
    Defines.Settings { id: settings }

  // MappingProperty { id: forward; path: "mapping.state." + deckId + ".forward"}
    QtObject {
        id: forward
        property string description: "Description"
        property var value: 0
    }

    property bool forwardVal: forward.value

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
                ToneInfoDetails {
                    id: header
                    label: "TONE"
                    width: 200
                    header: true
                }
            }

            Row {
                ToneInfoDetails {
                    id: bottomInfoDetails1
                    label: "0"
                    color: adjust != 0 ? "grey" : "white"
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails2
                    label: forwardVal ? "+1" : "-1"
                    color: forwardVal ? ((adjust == 1) && toneTimer.blink ? "white" : "lime") : ((adjust == -1) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails3
                    label: forwardVal ? "+2" : "-2"
                    color: forwardVal ? ((adjust == 2) && toneTimer.blink ? "white" : "lime") : ((adjust == -2) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails4
                    label: forwardVal ? "+3" : "-3"
                    color: forwardVal ? ((adjust == 3) && toneTimer.blink ? "white" : "lime") : ((adjust == -3) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
            }

            Row {
                ToneInfoDetails {
                    id: bottomInfoDetails5
                    label: forwardVal ? "+4" : "-4"
                    color: forwardVal ? ((adjust == 4) && toneTimer.blink ? "white" : "lime") : ((adjust == -4) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails6
                    label: forwardVal ? "+5" : "-5"
                    color: forwardVal ? ((adjust == 5) && toneTimer.blink ? "white" : "lime") : ((adjust == -5) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails7
                    label: forwardVal ? "+6" : "-6"
                    color: forwardVal ? ((adjust == 6) && toneTimer.blink ? "white" : "lime") : ((adjust == -6) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
                ToneInfoDetails {
                    id: bottomInfoDetails8
                    label: forwardVal ? "+7" : "-7"
                    color: forwardVal ? ((adjust == 7) && toneTimer.blink ? "white" : "lime") : ((adjust == -7) && toneTimer.blink ? "white" : "red")
                    width: 80
                }
            }
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
