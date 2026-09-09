pragma ComponentBehavior: Bound

import QtQuick
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import "../LateNightTheme"

Item {
    id: root

    enum MouseStatus {
        Normal,
        Bending,
        Scratching
    }

    required property string group
    readonly property int activeWaveformType: Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Simple || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Filtered || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.HSV || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.RGB || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Stacked ? Mixxx.Config.waveformType : Mixxx.WaveformDisplay.Type.RGB
    property bool splitStemTracks: false
    readonly property string zoomGroup: Mixxx.Config.waveformZoomSynchronization ? "[Channel1]" : group

    readonly property bool isPrimaryDeck: group === "[Channel1]" || group === "[Channel2]"
    readonly property color waveformBgColor: isPrimaryDeck
            ? LateNightTheme.waveformPrimaryBackgroundColor
            : LateNightTheme.waveformSecondaryBackgroundColor

    readonly property color cueColor: LateNightTheme.waveformCueColor
    readonly property color loopColor: LateNightTheme.waveformLoopColor
    readonly property color introOutroColor: LateNightTheme.waveformIntroOutroColor
    readonly property color playPosColor: LateNightTheme.waveformPlayPositionColor
    readonly property color beatAxesColor: LateNightTheme.waveformBeatAxesColor

    MixxxControls.WaveformDisplay {
        id: waveformDisplay

        anchors.fill: parent
        backgroundColor: root.waveformBgColor
        frameRate: Mixxx.Config.waveformFrameRate
        group: root.group
        options: Mixxx.Config.waveformOptions
        visible: Mixxx.Config.waveformEnabled
        zoom: zoomControl.value

        Behavior on zoom {
            SmoothedAnimation {
                duration: 500
                velocity: -1
            }
        }

        Mixxx.WaveformRendererEndOfTrack {
            color: LateNightTheme.waveformEndOfTrackWarningColor
            endOfTrackWarningTime: Mixxx.Config.waveformEndOfTrackWarningTime
        }
        Mixxx.WaveformRendererPreroll {
            color: LateNightTheme.waveformEndOfTrackWarningColor
        }
        Mixxx.WaveformRendererMarkRange {
            // Loop
            Mixxx.WaveformMarkRange {
                color: root.loopColor
                disabledColor: LateNightTheme.waveformDisabledMarkColor
                disabledOpacity: 0.6
                enabledControl: "loop_enabled"
                endControl: "loop_end_position"
                opacity: 0.8
                startControl: "loop_start_position"
            }
            // Intro
            Mixxx.WaveformMarkRange {
                color: root.introOutroColor
                durationTextColor: LateNightTheme.waveformMarkerTextColor
                durationTextLocation: 'after'
                startControl: "intro_start_position"
                endControl: "intro_end_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
                opacity: 0.1
            }
            // Outro
            Mixxx.WaveformMarkRange {
                color: root.introOutroColor
                durationTextColor: LateNightTheme.waveformMarkerTextColor
                durationTextLocation: 'before'
                startControl: "outro_start_position"
                endControl: "outro_end_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
                opacity: 0.1
            }
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: root.beatAxesColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Filtered
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: LateNightTheme.waveformFilteredHighColor
            lowColor: LateNightTheme.waveformFilteredLowColor
            midColor: LateNightTheme.waveformFilteredMidColor
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: root.beatAxesColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Stacked
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: LateNightTheme.waveformFilteredHighColor
            lowColor: LateNightTheme.waveformFilteredLowColor
            midColor: LateNightTheme.waveformFilteredMidColor
            stacked: true
        }
        Mixxx.WaveformRendererSimple {
            axesColor: root.beatAxesColor
            color: LateNightTheme.waveformFilteredHighColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Simple
            gain: Mixxx.Config.waveformVisualGainAll
        }
        Mixxx.WaveformRendererHSV {
            axesColor: root.beatAxesColor
            color: LateNightTheme.waveformFilteredHighColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.HSV
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
        }
        Mixxx.WaveformRendererRGB {
            axesColor: root.beatAxesColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.RGB
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: LateNightTheme.waveformFilteredHighColor
            lowColor: LateNightTheme.waveformFilteredLowColor
            midColor: LateNightTheme.waveformFilteredMidColor
        }
        Mixxx.WaveformRendererStem {
            gainAll: root.splitStemTracks ? 2.0 : 1.0
            opacity: Mixxx.Config.waveformStemOpacity
            outlineOpacity: Mixxx.Config.waveformStemOutlineOpacity
            reorderOnChange: Mixxx.Config.waveformStemReorderOnChange
            splitStemTracks: root.splitStemTracks || Mixxx.Config.waveformStemSplitTracks
        }
        Mixxx.WaveformRendererBeat {
            color: Qt.rgba(root.beatAxesColor.r,
                    root.beatAxesColor.g,
                    root.beatAxesColor.b,
                    Mixxx.Config.waveformBeatGridAlpha / 100)
        }
        Mixxx.WaveformRendererMark {
            playMarkerBackground: root.playPosColor
            playMarkerColor: root.playPosColor
            playMarkerPosition: Mixxx.Config.waveformPlayMarkerPosition
            untilMark.align: Mixxx.Config.waveformUntilMarkAlign === 0 ? Qt.AlignTop : Mixxx.Config.waveformUntilMarkAlign === 2 ? Qt.AlignBottom : Qt.AlignVCenter
            untilMark.showBeats: Mixxx.Config.waveformUntilMarkShowBeats
            untilMark.showTime: Mixxx.Config.waveformUntilMarkShowTime
            untilMark.textSize: Mixxx.Config.waveformUntilMarkTextPointSize

            defaultMark: Mixxx.WaveformMark {
                align: "bottom|right"
                color: LateNightTheme.waveformDefaultMarkColor
                disabledOpacity: 0.25
                endIcon: Qt.resolvedUrl("../../LateNight/classic/style/mark_jump_%1.svg")
                text: " %1 "
                textColor: LateNightTheme.waveformMarkerTextColor
            }

            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.cueColor
                control: "cue_point"
                text: 'CUE'
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.loopColor
                control: "loop_start_position"
                text: '↻'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_loop.svg")
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.loopColor
                control: "loop_end_position"
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.introOutroColor
                control: "intro_start_position"
                text: '◢'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.introOutroColor
                control: "intro_end_position"
                text: '◢'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.introOutroColor
                control: "outro_start_position"
                text: '◣'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_outro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: LateNightTheme.waveformMarkerTextColor
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.introOutroColor
                control: "outro_end_position"
                text: '◣'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_outro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
        }
    }

    Rectangle {
        id: leftFader
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 125
        enabled: false
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: root.waveformBgColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    Rectangle {
        id: rightFader
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 125
        enabled: false
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: root.waveformBgColor
            }
        }
    }

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
        group: root.zoomGroup
        key: "waveform_zoom"
        Component.onCompleted: {
            if (zoomControl.group === root.group) {
                zoomControl.value = Mixxx.Config.waveformDefaultZoom
            }
        }
    }

    MouseArea {
        property point mouseAnchor: Qt.point(0, 0)
        property int mouseStatus: LateNightWaveformDisplay.MouseStatus.Normal

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent

        onDoubleClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                root.splitStemTracks = !root.splitStemTracks;
            }
        }
        onPositionChanged: function(mouse) {
            const diff = mouse.x - mouseAnchor.x;
            switch (mouseStatus) {
            case LateNightWaveformDisplay.MouseStatus.Bending:
                {
                    const v = 0.5 + (diff / root.width);
                    wheelControl.parameter = Math.max(Math.min(v, 1), 0);
                    break;
                }
            case LateNightWaveformDisplay.MouseStatus.Scratching:
                scratchPositionControl.value = -mouse.x * waveformDisplay.audioSamplePerPixel * 2;
                break;
            }
        }
        onPressed: function(mouse) {
            mouseAnchor = Qt.point(mouse.x, mouse.y);
            if (mouse.button === Qt.LeftButton) {
                if (mouseStatus === LateNightWaveformDisplay.MouseStatus.Bending)
                    wheelControl.parameter = 0.5;

                mouseStatus = LateNightWaveformDisplay.MouseStatus.Scratching;
                scratchPositionControl.value = -mouse.x * waveformDisplay.audioSamplePerPixel * 2;
                scratchPositionEnableControl.value = 1;
            } else {
                if (mouseStatus === LateNightWaveformDisplay.MouseStatus.Scratching)
                    scratchPositionEnableControl.value = 0;

                wheelControl.parameter = 0.5;
                mouseStatus = LateNightWaveformDisplay.MouseStatus.Bending;
            }
        }
        onReleased: function(mouse) {
            switch (mouseStatus) {
            case LateNightWaveformDisplay.MouseStatus.Bending:
                wheelControl.parameter = 0.5;
                break;
            case LateNightWaveformDisplay.MouseStatus.Scratching:
                scratchPositionEnableControl.value = 0;
                scratchPositionControl.value = 0;
                break;
            }
            mouseStatus = LateNightWaveformDisplay.MouseStatus.Normal;
        }
        onWheel: function(mouse) {
            if (mouse.angleDelta.y < 0 && zoomControl.value > 1) {
                zoomControl.value -= 1;
            } else if (mouse.angleDelta.y > 0 && zoomControl.value < 10.0) {
                zoomControl.value += 1;
            }
        }
    }
}
