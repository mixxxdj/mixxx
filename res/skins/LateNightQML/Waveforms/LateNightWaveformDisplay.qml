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
    property bool splitStemTracks: false
    readonly property string zoomGroup: Mixxx.Config.waveformZoomSynchronization ? "[Channel1]" : group

    readonly property bool isPrimaryDeck: group === "[Channel1]" || group === "[Channel2]"
    readonly property color waveformBgColor: isPrimaryDeck ? "#0f0f0e" : "#001b23"

    readonly property string cueColor: LateNightTheme.isPaleMoon ? "#ff7a01" : "#ff001c"
    readonly property string loopColor: LateNightTheme.isPaleMoon ? "#00b400" : "#00ff00"
    readonly property string introOutroColor: LateNightTheme.isPaleMoon ? "#2c5c9a" : "#0000ff"
    readonly property color playPosColor: LateNightTheme.isPaleMoon ? "#00c6ff" : "#00c8ff"
    readonly property color beatAxesColor: LateNightTheme.isPaleMoon ? "#999999" : "#ffffff"

    MixxxControls.WaveformDisplay {
        anchors.fill: parent
        backgroundColor: root.waveformBgColor
        group: root.group
        zoom: zoomControl.value

        Behavior on zoom {
            SmoothedAnimation {
                duration: 500
                velocity: -1
            }
        }

        Mixxx.WaveformRendererEndOfTrack {
            color: '#ff8872'
            endOfTrackWarningTime: 30
        }
        Mixxx.WaveformRendererPreroll {
            color: '#ff8872'
        }
        Mixxx.WaveformRendererMarkRange {
            // Loop
            Mixxx.WaveformMarkRange {
                color: root.loopColor
                disabledColor: '#FFFFFF'
                disabledOpacity: 0.6
                enabledControl: "loop_enabled"
                endControl: "loop_end_position"
                opacity: 0.8
                startControl: "loop_start_position"
            }
            // Intro
            Mixxx.WaveformMarkRange {
                color: root.introOutroColor
                durationTextColor: '#ffffff'
                durationTextLocation: 'after'
                startControl: "intro_start_position"
                endControl: "intro_end_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
                opacity: 0.1
            }
            // Outro
            Mixxx.WaveformMarkRange {
                color: root.introOutroColor
                durationTextColor: '#ffffff'
                durationTextLocation: 'before'
                startControl: "outro_start_position"
                endControl: "outro_end_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
                opacity: 0.1
            }
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: root.beatAxesColor
            gainAll: 2.0
            gainHigh: 1.0
            gainLow: 1.0
            gainMid: 1.0
            highColor: '#D5C2A2'
            lowColor: '#2154D7'
            midColor: '#97632D'
        }
        Mixxx.WaveformRendererStem {
            gainAll: root.splitStemTracks ? 2.0 : 1.0
            splitStemTracks: root.splitStemTracks
        }
        Mixxx.WaveformRendererBeat {
            color: root.beatAxesColor
        }
        Mixxx.WaveformRendererMark {
            playMarkerBackground: root.playPosColor
            playMarkerColor: root.playPosColor
            untilMark.align: Qt.AlignBottom
            untilMark.showBeats: true
            untilMark.showTime: true
            untilMark.textSize: 11

            defaultMark: Mixxx.WaveformMark {
                align: "bottom|right"
                color: "#FF0000"
                disabledOpacity: 0.25
                endIcon: Qt.resolvedUrl("../../LateNight/classic/style/mark_jump_%1.svg")
                text: " %1 "
                textColor: "#FFFFFF"
            }

            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.cueColor
                control: "cue_point"
                text: 'CUE'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.loopColor
                control: "loop_start_position"
                text: '↻'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_loop.svg")
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.loopColor
                control: "loop_end_position"
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.introOutroColor
                control: "intro_start_position"
                text: '◢'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.introOutroColor
                control: "intro_end_position"
                text: '◢'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.introOutroColor
                control: "outro_start_position"
                text: '◣'
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_outro.svg")
                visibilityControl: "[Skin],show_intro_outro_cues"
                textColor: '#FFFFFF'
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
                scratchPositionControl.value = -diff * zoomControl.value * 200;
                break;
            }
        }
        onPressed: function(mouse) {
            mouseAnchor = Qt.point(mouse.x, mouse.y);
            if (mouse.button === Qt.LeftButton) {
                if (mouseStatus === LateNightWaveformDisplay.MouseStatus.Bending)
                    wheelControl.parameter = 0.5;

                mouseStatus = LateNightWaveformDisplay.MouseStatus.Scratching;
                scratchPositionControl.value = 0;
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
