import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import "Theme"

Item {
    id: root

    required property string group
    property bool splitStemTracks: false
    readonly property int activeWaveformType: Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Simple || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Filtered || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.HSV || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.RGB || Mixxx.Config.waveformType === Mixxx.WaveformDisplay.Type.Stacked ? Mixxx.Config.waveformType : Mixxx.WaveformDisplay.Type.RGB
    // Renderer factories are created once by QmlWaveformDisplay. Keep track
    // of the type that was used for the scene-graph stack so a change from
    // the legacy preferences dialog can recreate that stack explicitly.
    property int renderedWaveformType: -1
    readonly property string zoomGroup: Mixxx.Config.waveformZoomSynchronization ? "[Channel1]" : group

    Connections {
        target: Mixxx.Config

        function onWaveformTypeChanged() {
            // Defer until activeWaveformType has been reevaluated, then
            // rebuild the renderer list on the scene-graph thread.
            Qt.callLater(function() {
                if (root.activeWaveformType === root.renderedWaveformType) {
                    return;
                }
                root.renderedWaveformType = root.activeWaveformType;
                waveformDisplay.refreshRenderers();
            });
        }

        function onWaveformDefaultZoomChanged() {
            if (zoomControl.group === root.group) {
                zoomControl.value = Mixxx.Config.waveformDefaultZoom;
            }
        }
    }

    MixxxControls.WaveformDisplay {
        id: waveformDisplay

        anchors.fill: parent
        backgroundColor: "transparent"
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
            color: '#ff8872'
            endOfTrackWarningTime: Mixxx.Config.waveformEndOfTrackWarningTime
        }
        Mixxx.WaveformRendererPreroll {
            color: '#ff8872'
        }
        Mixxx.WaveformRendererMarkRange {
            // Loop
            Mixxx.WaveformMarkRange {
                color: '#00b400'
                disabledColor: '#FFFFFF'
                disabledOpacity: 0.6
                enabledControl: "loop_enabled"
                endControl: "loop_end_position"
                opacity: 0.7
                startControl: "loop_start_position"
            }
            // Intro
            Mixxx.WaveformMarkRange {
                color: '#2c5c9a'
                durationTextColor: '#ffffff'
                durationTextLocation: 'after'
                endControl: "intro_end_position"
                opacity: 0.6
                startControl: "intro_start_position"
            }
            // Outro
            Mixxx.WaveformMarkRange {
                color: '#2c5c9a'
                durationTextColor: '#ffffff'
                durationTextLocation: 'before'
                endControl: "outro_end_position"
                opacity: 0.6
                startControl: "outro_start_position"
            }
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: '#a1a1a1a1'
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Filtered
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: '#D5C2A2'
            lowColor: '#2154D7'
            midColor: '#97632D'
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: '#a1a1a1a1'
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Stacked
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: '#D5C2A2'
            lowColor: '#2154D7'
            midColor: '#97632D'
            stacked: true
        }
        Mixxx.WaveformRendererSimple {
            axesColor: '#a1a1a1a1'
            color: '#D5C2A2'
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Simple
            gain: Mixxx.Config.waveformVisualGainAll
        }
        Mixxx.WaveformRendererHSV {
            axesColor: '#a1a1a1a1'
            color: '#D5C2A2'
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.HSV
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
        }
        Mixxx.WaveformRendererRGB {
            axesColor: '#a1a1a1a1'
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.RGB
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: '#D5C2A2'
            lowColor: '#2154D7'
            midColor: '#97632D'
        }
        Mixxx.WaveformRendererStem {
            gainAll: root.splitStemTracks ? 2.0 : 1.0
            opacity: Mixxx.Config.waveformStemOpacity
            outlineOpacity: Mixxx.Config.waveformStemOutlineOpacity
            reorderOnChange: Mixxx.Config.waveformStemReorderOnChange
            splitStemTracks: root.splitStemTracks || Mixxx.Config.waveformStemSplitTracks
        }
        Mixxx.WaveformRendererBeat {
            color: Qt.rgba(161 / 255, 161 / 255, 161 / 255, Mixxx.Config.waveformBeatGridAlpha / 100)
        }
        Mixxx.WaveformRendererMark {
            playMarkerBackground: '#D9D9D9'
            playMarkerColor: '#D9D9D9'
            playMarkerPosition: Mixxx.Config.waveformPlayMarkerPosition
            untilMark.align: Mixxx.Config.waveformUntilMarkAlign === 0 ? Qt.AlignTop : Mixxx.Config.waveformUntilMarkAlign === 2 ? Qt.AlignBottom : Qt.AlignVCenter
            untilMark.showBeats: Mixxx.Config.waveformUntilMarkShowBeats
            untilMark.showTime: Mixxx.Config.waveformUntilMarkShowTime
            untilMark.textHeightLimit: Mixxx.Config.waveformUntilMarkTextHeightLimit
            untilMark.textSize: Mixxx.Config.waveformUntilMarkTextPointSize

            defaultMark: Mixxx.WaveformMark {
                align: "bottom|right"
                color: "#00d9ff"
                endIcon: Qt.resolvedUrl("images/jump_%1.svg")
                text: " %1 "
                textColor: "#1a1a1a"
            }

            Mixxx.WaveformMark {
                align: 'top|right'
                color: 'red'
                control: "cue_point"
                text: 'CUE'
                textColor: '#1a1a1a'
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: 'green'
                control: "loop_start_position"
                text: '↻'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'bottom|right'
                color: 'green'
                control: "loop_end_position"
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: 'blue'
                control: "intro_start_position"
                text: '◢'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: 'blue'
                control: "intro_end_position"
                text: '◢'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: 'blue'
                control: "outro_start_position"
                text: '◣'
                textColor: '#FFFFFF'
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: 'blue'
                control: "outro_end_position"
                text: '◣'
                textColor: '#FFFFFF'
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
            if (group == root.group) {
                value = Mixxx.Config.waveformDefaultZoom
            }
        }
    }
    // Scratching is driven by a PointHandler so that both waveforms can be
    // touched at the same time. A MouseArea only ever receives the single
    // mouse pointer that Qt synthesizes from the first touch point, which
    // makes two handed scratching impossible on a touchscreen.
    PointHandler {
        id: scratchHandler

        property real anchorX: 0

        acceptedButtons: Qt.LeftButton
        // Do not scratch while the user is pinching to zoom.
        enabled: !zoomPinchHandler.active

        onActiveChanged: {
            if (scratchHandler.active) {
                scratchHandler.anchorX = scratchHandler.point.position.x;
                scratchPositionControl.value = 0;
                scratchPositionEnableControl.value = 1;
            } else {
                scratchPositionEnableControl.value = 0;
                scratchPositionControl.value = 0;
            }
        }

        onPointChanged: {
            if (!scratchHandler.active) {
                return;
            }
            const diff = scratchHandler.point.position.x - scratchHandler.anchorX;
            scratchPositionControl.value = -diff * waveformDisplay.audioSamplePerPixel * 2;
        }
    }

    // Pinch to zoom, the touch equivalent of the mouse wheel below.
    PinchHandler {
        id: zoomPinchHandler

        // Tracks activeScale through a plain binding, which works no matter
        // which notify signal the underlying property uses.
        property real currentScale: zoomPinchHandler.activeScale
        property real zoomOnActivation: 1

        target: null

        onActiveChanged: {
            if (zoomPinchHandler.active) {
                zoomPinchHandler.zoomOnActivation = zoomControl.value;
            }
        }

        onCurrentScaleChanged: {
            if (!zoomPinchHandler.active) {
                return;
            }
            // Spreading the fingers shows more detail, i.e. a higher zoom
            // factor, which is what the waveform_zoom control counts.
            const zoom = zoomPinchHandler.zoomOnActivation * zoomPinchHandler.currentScale;
            zoomControl.value = Math.max(1, Math.min(10, zoom));
        }
    }

    // Pitch bending is bound to the right mouse button and therefore mouse
    // only. Touch points never reach this MouseArea because Qt synthesizes
    // left button presses from them.
    MouseArea {
        id: bendArea

        property real mouseAnchorX: 0

        acceptedButtons: Qt.RightButton
        anchors.fill: parent

        onDoubleClicked: root.splitStemTracks = !root.splitStemTracks
        onPositionChanged: mouse => {
            // Start at the middle of [0.0, 1.0], and emit values based on how far
            // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
            // this is tuned to 127.
            const v = 0.5 + ((mouse.x - bendArea.mouseAnchorX) / root.width);
            // clamp to [0.0, 1.0]
            wheelControl.parameter = Math.max(Math.min(v, 1), 0);
        }
        onPressed: mouse => {
            bendArea.mouseAnchorX = mouse.x;
            wheelControl.parameter = 0.5;
        }
        onReleased: wheelControl.parameter = 0.5
        onWheel: mouse => {
            if (mouse.angleDelta.y < 0 && zoomControl.value > 1) {
                zoomControl.value -= 1;
            } else if (mouse.angleDelta.y > 0 && zoomControl.value < 10.0) {
                zoomControl.value += 1;
            }
        }
    }
}
