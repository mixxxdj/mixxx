import QtQuick 2.15

import '../Defines'
import '../Widgets' as Widgets
import '../Overlays' as Overlays
import '../ViewModels' as ViewModels

import "../../../../../qml/" as Skin

Item {
    id: view
    property int deckId: deckInfo.deckId
    property string deckSizeState: "large"
    property bool showLoopSize: false
    property bool isInEditMode: false
    property string propertiesPath: ""
    property int zoomLevel: deckInfo.zoomLevel
    readonly property int minSampleWidth: 2048
    property int sampleWidth: minSampleWidth << zoomLevel
    property bool hideLoop: false
    property bool hideBPM: false
    property bool hideKey: false

    readonly property bool trackIsLoaded: deckInfo.isLoaded

  //--------------------------------------------------------------------------------------------------------------------

    required property var deckInfo

  //--------------------------------------------------------------------------------------------------------------------
  // WAVEFORM Position
  //------------------------------------------------------------------------------------------------------------------

    Skin.WaveformRow {
        id: singleWaveform
        group: `[Channel${view.deckId}]`
        x: 0
        width: 316
        // height: (settings.alwaysShowTempoInfo || deckInfo.adjustEnabled ? (settings.hideStripe ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38) : (!deckInfo.showBPMInfo ? (settings.hideStripe ? content.waveformHeight + display.secondRowHeight-13 : content.waveformHeight) : (settings.hideStripe ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38))) + (settings.hidePhase && settings.hidePhrase ? 16 : 0) + (!settings.hidePhase && !settings.hidePhrase ? -16 : 0)
        height: view.height

        shader.axesColor: 'transparent'

        zoomControlRatio: 100
        Behavior on height { PropertyAnimation { duration: 90} }
      // anchors.left:         parent.left
      // anchors.top:		  phase.bottom
    }

  //--------------------------------------------------------------------------------------------------------------------
  // Stem Color Indicators (Rectangles)
  //--------------------------------------------------------------------------------------------------------------------

    StemColorIndicators {
        id: stemColorIndicators
        deckId: view.deckId
        deckInfo: view.deckInfo
        anchors.fill: singleWaveform
        anchors.rightMargin:	 309
        visible: deckInfoModel.isStemDeck
        indicatorHeight: !settings.hidePhase && !settings.hidePhrase ? (deckInfo.showBPMInfo ? [19 , 19 , 19 , 20] : [27 , 27 , 27 , 27]) : (deckInfo.showBPMInfo ? [23 , 23 , 23 , 23] : [31 , 31 , 31 , 31])
    }

    Widgets.LoopSize {
        id: loopSize
        anchors.topMargin: 1
        anchors.fill: parent
        visible: (deckInfo.showLoopInfo || deckInfo.loopActive || settings.alwaysShowLoopSize) && !hideLoop
    }

    Widgets.KeyDisplay {
        id: keyDisplay
        anchors.topMargin: 1
        anchors.fill: parent
        visible: !hideKey
    }

    Widgets.BpmDisplay {
        id: bpmDisplay
        anchors.bottomMargin: 1
        anchors.top: singleWaveform.bottom
        anchors.fill: parent
        visible: !hideBPM && (!deckInfo.showBPMInfo && !settings.alwaysShowTempoInfo && !deckInfo.adjustEnabled) || settings.hideWaveforms
    }
}
