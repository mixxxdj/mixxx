import QtQuick 2.15

import '../Defines' as Defines

Item {
    id: tempoAdjust
    Defines.Margins {id: customMargins }
    Defines.Settings {id: settings}

    readonly property bool shift: deckInfo.shift

    property int deckId: 0

    function getHeader(headerID) {
        switch(headerID) {
            case 0:
                return "";
            case 1:
                return "Master BPM";
            case 2:
                return "BPM";
            case 3:
                return "Tempo";
            case 4:
                return "BPM Offset";
            case 5:
                return "Tempo Offset";
            case 6:
                return "Master Deck";
            case 7:
                return "Tempo Range";
            case 8:
                return "Key";
            case 9:
                return "Original BPM";
        }
    }

    function getValue(valueID) {
        switch(valueID) {
            case 0:
                return "";
            case 1:
                return deckInfo.masterBPMShort;
            case 2:
                return deckInfo.bpmString;
            case 3:
                return deckInfo.tempoStringPer;
            case 4:
                return (deckInfo.masterDeck == tempoAdjust.deckId) ? "0.00" : deckInfo.bpmOffset;
            case 5:
                return (deckInfo.masterDeck == tempoAdjust.deckId) ? "0.00%" : deckInfo.tempoNeededString;
            case 6:
                return deckInfo.masterDeckLetter
            case 7:
                return deckInfo.tempoRange
            case 8:
                return deckInfo.hasKey && (deckInfo.keyAdjustString != "-0") && (deckInfo.keyAdjustString != "+0") ? (settings.camelotKey ? utils.camelotConvert(deckInfo.keyString) : deckInfo.keyString) + deckInfo.keyAdjustString : deckInfo.hasKey ? (settings.camelotKey ? utils.camelotConvert(deckInfo.keyString) : deckInfo.keyString) : "No key";
            case 9:
                return deckInfo.songBPM
        }
    }

    function getColor(valueID) {
        switch(valueID) {
            case 0:
                return "white";
            case 1:
                return settings.enableMasterBpmTextColor ? ((deckInfo.masterDeck == deckInfo.deckId) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 2:
                return settings.enableBpmTextColor ? ((deckInfo.masterDeck == tempoAdjust.deckId) || ((deckInfo.bpmOffset <= 0.05) && (deckInfo.bpmOffset >= - 0.05)) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 3:
                return settings.enableTempoTextColor ? ((deckInfo.tempoString <= 0.05) && (deckInfo.tempoString >= - 0.05) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 4:
                return settings.enableBpmOffsetTextColor ? ((deckInfo.masterDeck == tempoAdjust.deckId) || ((deckInfo.bpmOffset <= 0.05) && (deckInfo.bpmOffset >= - 0.05)) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 5:
                return settings.enableTempoOffsetTextColor ? ((deckInfo.masterDeck == tempoAdjust.deckId) || ((deckInfo.tempoNeededVal <= 0.05) && (deckInfo.tempoNeededVal >= - 0.05)) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 6:
                return settings.enableMasterDeckTextColor ? ((deckInfo.masterDeck == deckInfo.deckId) ? colors.loopActiveColor : colors.lightOrange) : "white";
            case 7:
                return "white"
            case 8:
                return deckInfo.isKeyLockOn ? colors.musicalKeyColors[deckInfo.keyIndex] : "white"
            case 9:
                return "white"
        }
    }

    Rectangle {
        id: tempoBackground
        width: 320
        height: 38

        color: colors.grayBackground
  // headline
        Text {
            anchors.top: tempoBackground.top
            anchors.topMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 3
            font.pixelSize: 15
            color: settings.accentColor
            text: shift ? getHeader(settings.tempoDisplayLeftShift) : getHeader(settings.tempoDisplayLeft)
        }

  // value
        Text {
            anchors.bottom: tempoBackground.bottom
            anchors.bottomMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 3
            font.pixelSize: 20
            font.family: "Pragmatica"
            color: 			 shift ? getColor(settings.tempoDisplayLeftShift) : getColor(settings.tempoDisplayLeft)
            text: shift ? getValue(settings.tempoDisplayLeftShift) : getValue(settings.tempoDisplayLeft)
        }

  // headline
        Text {
            anchors.top: tempoBackground.top
            anchors.topMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 100
            font.pixelSize: 15
            color: settings.accentColor
            text: shift ? getHeader(settings.tempoDisplayCenterShift) : getHeader(settings.tempoDisplayCenter)
        }

  // value
        Text {

            anchors.bottom: tempoBackground.bottom
            anchors.bottomMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 100
            font.pixelSize: 20
            font.family: "Pragmatica"
            color: shift ? getColor(settings.tempoDisplayCenterShift) : getColor(settings.tempoDisplayCenter)
            text: shift ? getValue(settings.tempoDisplayCenterShift) : getValue(settings.tempoDisplayCenter)
        }

  // headline
        Text {
            anchors.top: tempoBackground.top
            anchors.topMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 216
            font.pixelSize: 15
            color: settings.accentColor
            text: shift ? getHeader(settings.tempoDisplayRightShift) : getHeader(settings.tempoDisplayRight)
        }

  // value
        Text {

            anchors.bottom: tempoBackground.bottom
            anchors.bottomMargin: 0
            anchors.left: tempoBackground.left
            anchors.leftMargin: 216
            font.pixelSize: 20
            font.family: "Pragmatica"
            color: shift ? getColor(settings.tempoDisplayRightShift) : getColor(settings.tempoDisplayRight)
            text: shift ? getValue(settings.tempoDisplayRightShift) : getValue(settings.tempoDisplayRight)
        }
    }

    Rectangle {
        width: 1
        height: 38
        color: "#88ffffff"
        anchors.top: tempoBackground.top
        anchors.left: tempoBackground.left
        anchors.leftMargin: 97
    }

    Rectangle {
        width: 1
        height: 38
        color: "#88ffffff"
        anchors.top: tempoBackground.top
        anchors.left: tempoBackground.left
        anchors.leftMargin: 213
    }
}
