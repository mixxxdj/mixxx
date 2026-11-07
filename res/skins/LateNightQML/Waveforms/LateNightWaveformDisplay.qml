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
    // Renderer factories are created once by QmlWaveformDisplay. Keep track
    // of the type that was used for the scene-graph stack so a change from
    // the legacy preferences dialog can recreate that stack explicitly.
    property int renderedWaveformType: -1
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
    readonly property color waveformSignalColor: isPrimaryDeck ? LateNightTheme.waveformPrimarySignalColor : LateNightTheme.waveformSecondarySignalColor
    readonly property bool trackLoaded: trackLoadedControl.value > 0
    readonly property bool passthroughEnabled: passthroughControl.value > 0

    signal splitStemTracksToggleRequested

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Mixxx.ControlProxy {
        id: passthroughControl

        group: root.group
        key: "passthrough"
    }

    Rectangle {
        anchors.fill: parent
        color: root.waveformBgColor
        visible: root.passthroughEnabled
    }

    Connections {
        target: Mixxx.Config

        function onWaveformTypeChanged() {
            // The legacy QWidget preferences page writes UserSettings
            // directly and then emits the proxy notification. Defer until
            // the binding for activeWaveformType has settled, then rebuild
            // the renderer list on the scene-graph thread.
            Qt.callLater(function() {
                if (root.activeWaveformType === root.renderedWaveformType) {
                    return;
                }
                root.renderedWaveformType = root.activeWaveformType;
                waveformDisplay.refreshRenderers();
            });
        }

        function onWaveformDefaultZoomChanged() {
            // Component.onCompleted initializes this value only once. Make
            // changing the default zoom in the legacy page affect existing
            // decks as well, while leaving a shared [Channel1] zoom alone on
            // secondary decks.
            if (zoomControl.group === root.group) {
                zoomControl.value = Mixxx.Config.waveformDefaultZoom;
            }
        }
    }

    MixxxControls.WaveformDisplay {
        id: waveformDisplay

        visible: !root.passthroughEnabled && Mixxx.Config.waveformEnabled
        anchors.fill: parent
        backgroundColor: root.waveformBgColor
        frameRate: Mixxx.Config.waveformFrameRate
        group: root.group
        options: Mixxx.Config.waveformOptions
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
            color: root.waveformSignalColor
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
                endControl: "intro_end_position"
                opacity: 0.1
                startControl: "intro_start_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
            // Outro
            Mixxx.WaveformMarkRange {
                color: root.introOutroColor
                durationTextColor: LateNightTheme.waveformMarkerTextColor
                durationTextLocation: 'before'
                endControl: "outro_end_position"
                opacity: 0.1
                startControl: "outro_start_position"
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
        }
        Mixxx.WaveformRendererFiltered {
            axesColor: root.beatAxesColor
            enabled: root.activeWaveformType === Mixxx.WaveformDisplay.Type.Filtered
            gainAll: Mixxx.Config.waveformVisualGainAll
            gainHigh: Mixxx.Config.waveformVisualGainHigh
            gainLow: Mixxx.Config.waveformVisualGainLow
            gainMid: Mixxx.Config.waveformVisualGainMedium
            highColor: root.waveformSignalColor
            lowColor: root.waveformSignalColor
            midColor: root.waveformSignalColor
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
            highColor: LateNightTheme.overviewRgbHighColor
            lowColor: LateNightTheme.overviewRgbLowColor
            midColor: LateNightTheme.overviewRgbMidColor
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
            playMarkerBackground: root.trackLoaded ? root.playPosColor : "transparent"
            playMarkerColor: root.trackLoaded ? root.playPosColor : "transparent"
            playMarkerPosition: Mixxx.Config.waveformPlayMarkerPosition
            untilMark.align: Mixxx.Config.waveformUntilMarkAlign === 0 ? Qt.AlignTop : Mixxx.Config.waveformUntilMarkAlign === 2 ? Qt.AlignBottom : Qt.AlignVCenter
            untilMark.showBeats: Mixxx.Config.waveformUntilMarkShowBeats
            untilMark.showTime: Mixxx.Config.waveformUntilMarkShowTime
            untilMark.textHeightLimit: Mixxx.Config.waveformUntilMarkTextHeightLimit
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
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_loop.svg")
                text: '↻'
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
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                text: '◢'
                textColor: LateNightTheme.waveformMarkerTextColor
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.introOutroColor
                control: "intro_end_position"
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_intro.svg")
                text: '◢'
                textColor: LateNightTheme.waveformMarkerTextColor
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
            Mixxx.WaveformMark {
                align: 'top|right'
                color: root.introOutroColor
                control: "outro_start_position"
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_outro.svg")
                text: '◣'
                textColor: LateNightTheme.waveformMarkerTextColor
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
            Mixxx.WaveformMark {
                align: 'top|left'
                color: root.introOutroColor
                control: "outro_end_position"
                icon: Qt.resolvedUrl("../../LateNight/classic/style/mark_outro.svg")
                text: '◣'
                visibilityControl: "[Skin],show_intro_outro_cues"
            }
        }
    }
    Item {
        id: passthroughLayer

        anchors.fill: parent
        enabled: false
        visible: root.passthroughEnabled
        z: 1

        Text {
            anchors.centerIn: parent
            color: LateNightTheme.passthroughLabelColor
            font.bold: true
            font.family: "Open Sans"
            font.pixelSize: Math.max(1, Math.min(25, Math.floor(parent.height * 0.8)))
            text: qsTr("Passthrough")
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
                zoomControl.value = Mixxx.Config.waveformDefaultZoom;
            }
        }
    }
    TapHandler {
        acceptedButtons: Qt.RightButton
        grabPermissions: PointerHandler.CanTakeOverFromAnything

        onDoubleTapped: root.splitStemTracksToggleRequested()
    }
    MouseArea {
        property point mouseAnchor: Qt.point(0, 0)
        property int mouseStatus: LateNightWaveformDisplay.MouseStatus.Normal

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent

        onPositionChanged: function (mouse) {
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
        onPressed: function (mouse) {
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
        onReleased: function (mouse) {
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
        onWheel: function (mouse) {
            if (mouse.angleDelta.y < 0 && zoomControl.value > 1) {
                zoomControl.value -= 1;
            } else if (mouse.angleDelta.y > 0 && zoomControl.value < 10.0) {
                zoomControl.value += 1;
            }
        }
    }
}
