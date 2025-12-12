import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import "Theme"

Item {
    id: root

    enum MouseStatus {
        Normal,
        Bending,
        Scratching
    }

    required property string group
    property bool splitStemTracks: false
    readonly property string zoomGroup: Mixxx.Config.waveformZoomSynchronization() ? "[Channel1]" : group

    MixxxControls.WaveformDisplay {
        anchors.fill: parent
        backgroundColor: "transparent"
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
            gainAll: 1.0
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
            color: '#a1a1a1a1'
        }
        Mixxx.WaveformRendererMark {
            playMarkerBackground: '#D9D9D9'
            playMarkerColor: '#D9D9D9'
            untilMark.align: Qt.AlignBottom
            untilMark.showBeats: true
            untilMark.showTime: true
            untilMark.textSize: 11

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
                value = Mixxx.Config.waveformDefaultZoom();
            }
        }
    }
    MouseArea {
        property point mouseAnchor: Qt.point(0, 0)
        property int mouseStatus: WaveformDisplay.MouseStatus.Normal

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent

        onDoubleClicked: {
            if (mouse.button == Qt.RightButton) {
                root.splitStemTracks = !root.splitStemTracks;
            }
        }
        onPositionChanged: {
            const diff = mouse.x - mouseAnchor.x;
            switch (mouseStatus) {
            case WaveformDisplay.MouseStatus.Bending:
                {
                    // Start at the middle of [0.0, 1.0], and emit values based on how far
                    // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
                    // this is tuned to 127.
                    const v = 0.5 + (diff / root.width);
                    // clamp to [0.0, 1.0]
                    wheelControl.parameter = Math.max(Math.min(v, 1), 0);
                    break;
                }
                ;
            case WaveformDisplay.MouseStatus.Scratching:
                // TODO: Calculate position properly
                scratchPositionControl.value = -diff * zoomControl.value * 200;
                break;
            }
        }
        onPressed: {
            mouseAnchor = Qt.point(mouse.x, mouse.y);
            if (mouse.button == Qt.LeftButton) {
                if (mouseStatus == WaveformDisplay.MouseStatus.Bending)
                    wheelControl.parameter = 0.5;

                mouseStatus = WaveformDisplay.MouseStatus.Scratching;
                scratchPositionControl.value = 0;
                scratchPositionEnableControl.value = 1;
            } else {
                if (mouseStatus == WaveformDisplay.MouseStatus.Scratching)
                    scratchPositionEnableControl.value = 0;

                wheelControl.parameter = 0.5;
                mouseStatus = WaveformDisplay.MouseStatus.Bending;
            }
        }
        onReleased: {
            switch (mouseStatus) {
            case WaveformDisplay.MouseStatus.Bending:
                wheelControl.parameter = 0.5;
                break;
            case WaveformDisplay.MouseStatus.Scratching:
                scratchPositionEnableControl.value = 0;
                scratchPositionControl.value = 0;
                break;
            }
            mouseStatus = WaveformDisplay.MouseStatus.Normal;
        }
        onWheel: mouse => {
            if (mouse.angleDelta.y < 0 && zoomControl.value > 1) {
                zoomControl.value -= 1;
            } else if (mouse.angleDelta.y > 0 && zoomControl.value < 10.0) {
                zoomControl.value += 1;
            }
        }
    }
}
