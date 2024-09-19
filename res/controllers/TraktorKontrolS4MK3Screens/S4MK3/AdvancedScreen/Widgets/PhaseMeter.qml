import QtQuick 2.5
import '../Defines' as Defines

Item {
    id: widget

    height: 16

    function colorForPhase(phase) {
        switch (phase) {
            case 0: return colors.red;
            case 1: return colors.darkOrange;
            case 2: return colors.lightOrange;
            case 3: return colors.phaseColor;
            case 4: return colors.yellow;
            case 5: return colors.lime;
            case 6: return colors.green;
            case 7: return colors.mint;
            case 8: return colors.cyan;
            case 9: return colors.turquoise;
            case 10: return colors.blue;
            case 11: return colors.plum;
            case 12: return colors.violet;
            case 13: return colors.purple;
            case 14: return colors.magenta;
            case 15: return colors.fuchsia;
            case 16: return colors.colorWhite;
        }
        return colors.lightOrange;
    }

    property real phase: 0.0

    Defines.Settings {id: settings}
    property int phaseAColor: settings.phaseAColor
    property int phaseBColor: settings.phaseBColor
    property int phaseCColor: settings.phaseCColor
    property int phaseDColor: settings.phaseDColor
    property int deckId:	 deckInfo.deckId

    property color phaseColor: colorForPhase(deckId == 1 ? phaseAColor : deckId == 2 ? phaseBColor : deckId == 3 ? phaseCColor : phaseDColor)
    property color phaseHeadColor: "#FCB262"
    property color separatorColor: "#88ffffff"
    property color backgroundColor: colors.grayBackground
    property real phasePosition: parent.width * (0.5 + widget.phase)
    property real phaseBarWidth: parent.width * Math.abs(widget.phase)

    // Background
    Rectangle {
        anchors.fill: parent
        color: widget.backgroundColor
    }

    // Phase Bar
    Rectangle {
        color: widget.phaseColor
        height: parent.height
        width: phaseBarWidth
        x: widget.phase < 0 ? widget.phasePosition : (parent.width/2)
    }

    // Phase Head
    Rectangle {
        color: widget.phaseHeadColor
        height: parent.height
        width: 1
        x: widget.phase < 0 ? widget.phasePosition : (widget.phasePosition - width)
        visible: Math.round(phaseBarWidth) !== 0 // hide phase head when phase is 0
    }

    // Separator at 0.25
    Rectangle {
        color: widget.separatorColor
        height: parent.height
        width: 1
        x: parent.width * 0.25 - 1
    }

    // center Separator
    Rectangle {
        color: widget.separatorColor
        height: parent.height
        width: 1
        x: parent.width * 0.50 - 1
    }

    // Separator at 0.75
    Rectangle {
        color: widget.separatorColor
        height: parent.height
        width: 1
        x: parent.width * 0.75 - 1
    }
}
