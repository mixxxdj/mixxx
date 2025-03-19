import QtQuick 2.5
import QtQuick.Layouts 1.1
import '../Waveform' as WF
import '../Overlays' as Overlays

import '../Widgets' as Widgets

//----------------------------------------------------------------------------------------------------------------------
//  Track Screen View - UI of the screen for track
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: display
    Dimensions {id: dimensions}

  // MODEL PROPERTIES //
    required property var deckInfo
    property int deckId: 1
    property real boxesRadius: dimensions.cornerRadius
    property real infoBoxesWidth: dimensions.infoBoxesWidth +4
    property real firstRowHeight: dimensions.firstRowHeight
    property real secondRowHeight: dimensions.secondRowHeight
    property real spacing: dimensions.spacing-3
    property real screenTopMargin: dimensions.screenTopMargin
    property real screenLeftMargin: dimensions.screenLeftMargin-2

    width: 320
    height: 240

    Rectangle {
        id: displayBackground
        anchors.fill: parent
        color: colors.defaultBackground
    }

    Image {
        id: emptyTrackDeckImage
        anchors.fill: parent
        visible: deckInfo.showLogo

        source: engine.getSetting("idleBackground") || "../../../../../images/templates/logo_mixxx.png"
        fillMode: Image.PreserveAspectFit
    }

    ColumnLayout {
        id: content
        spacing: display.spacing

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: display.screenTopMargin
        anchors.leftMargin: display.screenLeftMargin

	// FIRST ROW //
        RowLayout {
            id: firstRow

            spacing: 1

		// DECK HEADER //
            Widgets.DeckHeader {
                id: deckHeader

                deckInfo: display.deckInfo

                title: deckInfo.headerEnabled ? deckInfo.headerTextShort : deckInfo.titleString
                artist: deckInfo.headerEnabled ? deckInfo.headerTextLong : deckInfo.artistString

                height: display.firstRowHeight-6
                width: deckInfo.headerEnabled ? 4*(display.infoBoxesWidth/2+1)+1 : 3*(display.infoBoxesWidth/2+1)+3
            }

	  // TIME DISPLAY //
            Item {
                id: timeBox2
                width: (display.infoBoxesWidth/2+1)
                height: display.firstRowHeight-6

                Rectangle {
                    anchors.fill: parent
                    color: trackEndBlinkTimer2.blink ? colors.colorRed : colors.colorDeckGrey
                    radius: display.boxesRadius
                    visible: !deckInfo.headerEnabled
                }

                Text {
                    text: settings.timeBox == 0 ? deckInfo.remainingTimeString : settings.timeBox == 1 ? deckInfo.elapsedTimeString : settings.timeBox == 2 ? deckInfo.timeToCue : settings.timeBox == 3 ? deckInfo.beats : settings.timeBox == 4 ? deckInfo.beatsAlt : settings.timeBox == 5 ? deckInfo.beatsToCue : settings.timeBox == 6 ? deckInfo.beatsToCueAlt : deckInfo.remainingTimeString
                    font.pixelSize: 22
                    font.family: "Roboto"
                    font.weight: Font.Medium
                    color: settings.timeTextColorChange && trackEndBlinkTimer2.blink ? "black" : "white"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: !deckInfo.shift && !deckInfo.headerEnabled
                }

                Text {
                    text: settings.timeBoxShift == 0 ? deckInfo.remainingTimeString : settings.timeBoxShift == 1 ? deckInfo.elapsedTimeString : settings.timeBoxShift == 2 ? deckInfo.timeToCue : settings.timeBoxShift == 3 ? deckInfo.beats : settings.timeBoxShift == 4 ? deckInfo.beatsAlt : settings.timeBoxShift == 5 ? deckInfo.beatsToCue : settings.timeBoxShift == 6 ? deckInfo.beatsToCueAlt : deckInfo.remainingTimeString
                    font.pixelSize: 22
                    font.family: "Roboto"
                    font.weight: Font.Medium
                    color: settings.timeTextColorChange && trackEndBlinkTimer2.blink ? "black" : "white"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: deckInfo.shift && !deckInfo.headerEnabled
                }

                Timer {
                    id: trackEndBlinkTimer2
                    property bool blink: false

                    interval: 500
                    repeat: true
                    running: deckInfo.trackEndWarning

                    onTriggered: {
                        blink = !blink;
                    }

                    onRunningChanged: {
                        blink = running;
                    }
                }
            }
        }

    // PHASE METER //
        Widgets.PhaseMeter {
            id: phase
            height: settings.hidePhase ? 0 : 16
            width: 317
            visible: deckInfo.isLoaded

            phase: deckInfo.phase
        }

	//WAVEFORM

        property string deckSizeState: "large"
        readonly property int waveformHeight: 129
        property bool isInEditMode: false
        property bool showLoopSize: true
        property string propertiesPath: ""

        WF.WaveformContainer {
            id: waveformContainer

            deckInfo: display.deckInfo

            deckId: deckInfo.deckId
            deckSizeState: content.deckSizeState
            propertiesPath: content.propertiesPath

      // anchors.left:         parent.left
            width: 316
      // anchors.top:		  phase.bottom
            showLoopSize: content.showLoopSize
            isInEditMode: content.isInEditMode

      // the height of the waveform is defined as the remaining space of deckHeight - stripe.height - spacerWaveStripe.height
            height: (settings.alwaysShowTempoInfo || deckInfo.adjustEnabled ? (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38) : (!deckInfo.showBPMInfo ? (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-13 : content.waveformHeight) : (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38))) + (settings.hidePhase && settings.hidePhrase ? 16 : 0) + (!settings.hidePhase && !settings.hidePhrase ? -16 : 0)
            visible: deckInfo.isLoaded && !settings.hideWaveforms

            Behavior on height { PropertyAnimation { duration: 90} }
        }
    }

    WF.WaveformOverview {
        height: settings.hideWaveformOverview ? 0 : settings.hideWaveforms ? 150 : display.secondRowHeight-13
        width: 314
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: display.top
        anchors.topMargin: settings.hideWaveforms ? 90 : 178
    }

    Overlays.TopControls {
        id: fx1
        fxUnit: 0
        showHideState: (deckInfo.showFx1 && settings.fxOverlays && !settings.hideEffectsOverlay1) || (deckInfo.padsModeFx1 && (settings.fx1unit == 1)) || (deckInfo.padsModeFx2 && (settings.fx2unit == 1)) ? "show" : "hide"
    }

    Overlays.TopControls {
        id: fx2
        fxUnit: 1
        showHideState: deckInfo.showFx2 && settings.fxOverlays && !settings.hideEffectsOverlay1 || (deckInfo.padsModeFx1 && (settings.fx1unit == 2)) || (deckInfo.padsModeFx2 && (settings.fx2unit == 2)) ? "show" : "hide"
    }

    Overlays.TopControls {
        id: fx3
        fxUnit: 2
        showHideState: deckInfo.showFx3 && settings.fxOverlays && !settings.hideEffectsOverlay2 || (deckInfo.padsModeFx1 && (settings.fx1unit == 3)) || (deckInfo.padsModeFx2 && (settings.fx2unit == 3)) ? "show" : "hide"
    }

    Overlays.QuickFXSelector {
        deckInfo: display.deckInfo
    }

    Overlays.TopControls {
        id: fx4
        fxUnit: 3
        showHideState: (deckInfo.showFx4 && settings.fxOverlays && !settings.hideEffectsOverlay2 || (deckInfo.padsModeFx1 && (!settings.fx1unit == 4)) ||(deckInfo.padsModeFx2 && (settings.fx2unit == 4))) ? "show" : "hide"
    }

    Widgets.TempoAdjust {
        id: tempoInfo
        deckId: deckInfo.deckId
        height: 38
        y: settings.hideWaveformOverview ? 197 : 140
        visible: (deckInfo.isLoaded ? (settings.alwaysShowTempoInfo || deckInfo.adjustEnabled ? true : deckInfo.showBPMInfo) : false) && !settings.hideWaveforms
    }

    Widgets.TempoAdjust {
        id: tempoInfo2
        deckId: deckInfo.deckId
        height: 38
        y: 50
        visible: settings.hideWaveforms
    }
}
