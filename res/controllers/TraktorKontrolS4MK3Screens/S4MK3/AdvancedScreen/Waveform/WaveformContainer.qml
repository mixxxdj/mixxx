import QtQuick 2.15

import '../Defines'
import '../Widgets' as Widgets
import '../Overlays' as Overlays
import '../ViewModels' as ViewModels

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

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

    Mixxx.ControlProxy {
        id: scratchPositionEnableControl

        group: root.group
        key: "scratch_position_enable"
    }

    Mixxx.ControlProxy {
        id: scratchPositionControl

        group: root.group
        key: "scratch_position"
    }

    Mixxx.ControlProxy {
        id: wheelControl

        group: root.group
        key: "wheel"
    }

    Mixxx.ControlProxy {
        id: rateRatioControl

        group: root.group
        key: "rate_ratio"
    }

    Mixxx.ControlProxy {
        id: zoomControl

        group: root.group
        key: "waveform_zoom"
    }

    MixxxControls.WaveformDisplay {
        id: singleWaveform
        group: `[Channel${view.deckId}]`
        x: 0
        width: 316
        height: (settings.alwaysShowTempoInfo || deckInfo.adjustEnabled ? (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38) : (!deckInfo.showBPMInfo ? (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-13 : content.waveformHeight) : (settings.hideWaveformOverview ? content.waveformHeight + display.secondRowHeight-51 : content.waveformHeight-38))) + (settings.hidePhase && settings.hidePhrase ? 16 : 0) + (!settings.hidePhase && !settings.hidePhrase ? -16 : 0)

        Behavior on height { PropertyAnimation { duration: 90} }
        anchors.fill: parent
        zoom: zoomControl.value
        backgroundColor: "#36000000"

        Mixxx.WaveformRendererEndOfTrack {
            color: 'blue'
            endOfTrackWarningTime: 30
        }

        Mixxx.WaveformRendererPreroll {
            color: '#998977'
        }

        Mixxx.WaveformRendererMarkRange {
            // <!-- Loop -->
            Mixxx.WaveformMarkRange {
                startControl: "loop_start_position"
                endControl: "loop_end_position"
                enabledControl: "loop_enabled"
                color: '#00b400'
                opacity: 0.7
                disabledColor: '#FFFFFF'
                disabledOpacity: 0.6
            }
            // <!-- Intro -->
            Mixxx.WaveformMarkRange {
                startControl: "intro_start_position"
                endControl: "intro_end_position"
                color: '#2c5c9a'
                opacity: 0.6
                durationTextColor: '#ffffff'
                durationTextLocation: 'after'
            }
            // <!-- Outro -->
            Mixxx.WaveformMarkRange {
                startControl: "outro_start_position"
                endControl: "outro_end_position"
                color: '#2c5c9a'
                opacity: 0.6
                durationTextColor: '#ffffff'
                durationTextLocation: 'before'
            }
        }

        Mixxx.WaveformRendererRGB {
            axesColor: '#00ffffff'
            lowColor: 'red'
            midColor: 'green'
            highColor: 'blue'

            gainAll: 1.5
            gainLow: 1.0
            gainMid: 1.0
            gainHigh: 1.0
        }

        Mixxx.WaveformRendererStem {
            gainAll: 1.5
        }

        Mixxx.WaveformRendererBeat {
            color: settings.hideBeatgrid ? 'transparent' : Qt.rgba(0.81, 0.81, 0.81, settings.beatgridVisibility)
        }

        Mixxx.WaveformRendererMark {
            playMarkerColor: 'cyan'
            playMarkerBackground: 'transparent'
            defaultMark: Mixxx.WaveformMark {
                align: "bottom|right"
                color: "#FF0000"
                textColor: "#FFFFFF"
                text: " %1 "
            }

            untilMark.showTime: settings.showTimeToCue
            untilMark.showBeats: settings.showBeatToCue
            untilMark.align: settings.distanceToCueAlignment == "bottom" ? Qt.AlignBottom : settings.distanceToCueAlignment == "top" ? Qt.AlignTop : Qt.AlignCenter
            untilMark.textSize: settings.distanceToCueFontSize

            Mixxx.WaveformMark {
                control: "cue_point"
                text: 'C'
                align: 'top|right'
                color: 'red'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "loop_start_position"
                text: '↻'
                align: 'top|left'
                color: 'green'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "loop_end_position"
                align: 'bottom|right'
                color: 'green'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "intro_start_position"
                align: 'top|right'
                color: 'blue'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "intro_end_position"
                text: '◢'
                align: 'top|left'
                color: 'blue'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "outro_start_position"
                text: '◣'
                align: 'top|right'
                color: 'blue'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                control: "outro_end_position"
                align: 'top|left'
                color: 'blue'
                textColor: '#FFFFFF'
            }
        }
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
