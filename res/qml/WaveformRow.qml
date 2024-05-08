import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.14
import QtQuick.Shapes 1.12
import "Theme"

Item {
    id: root

    enum MouseStatus {
        Normal,
        Bending,
        Scratching
    }

    property string group // required
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)

    Item {
        id: waveformContainer

        property real duration: samplesControl.value / sampleRateControl.value

        anchors.fill: parent
        clip: true

        Mixxx.ControlProxy {
            id: samplesControl

            group: root.group
            key: "track_samples"
        }

        Mixxx.ControlProxy {
            id: sampleRateControl

            group: root.group
            key: "track_samplerate"
        }

        Mixxx.ControlProxy {
            id: playPositionControl

            group: root.group
            key: "playposition"
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

        Mixxx.ControlProxy {
            id: introStartPosition

            group: root.group
            key: "intro_start_position"
        }

        Mixxx.ControlProxy {
            id: introEndPosition

            group: root.group
            key: "intro_end_position"
        }

        Mixxx.ControlProxy {
            id: outroStartPosition

            group: root.group
            key: "outro_start_position"
        }

        Mixxx.ControlProxy {
            id: outroEndPosition

            group: root.group
            key: "outro_end_position"
        }

        Mixxx.ControlProxy {
            id: loopStartPosition

            group: root.group
            key: "loop_start_position"
        }

        Mixxx.ControlProxy {
            id: loopEndPosition

            group: root.group
            key: "loop_end_position"
        }

        Mixxx.ControlProxy {
            id: loopEnabled

            group: root.group
            key: "loop_enabled"
        }

        Mixxx.ControlProxy {
            id: mainCuePosition

            group: root.group
            key: "cue_point"
        }

        Item {
            id: waveform

            property real effectiveZoomFactor: (1 / rateRatioControl.value) * (100 / zoomControl.value)

            width: waveformContainer.duration * effectiveZoomFactor
            height: parent.height
            x: playMarker.screenPosition * waveformContainer.width - playPositionControl.value * width
            visible: root.deckPlayer.isLoaded

            WaveformShader {
                group: root.group
                anchors.fill: parent
            }

            Shape {
                id: preroll

                property real triangleHeight: waveform.height
                property real triangleWidth: 0.25 * waveform.effectiveZoomFactor
                property int numTriangles: Math.ceil(width / triangleWidth)

                anchors.top: waveform.top
                anchors.right: waveform.left
                width: Math.max(0, waveform.x)
                height: waveform.height

                ShapePath {
                    strokeColor: Theme.waveformPrerollColor
                    strokeWidth: 1
                    fillColor: "transparent"

                    PathMultiline {
                        paths: {
                            let p = [];
                            for (let i = 0; i < preroll.numTriangles; i++) {
                                p.push([
                                        Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2),
                                        Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, 0),
                                        Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, preroll.triangleHeight),
                                        Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2),
                                ]);
                            }
                            return p;
                        }
                    }
                }
            }

            Shape {
                id: postroll

                property real triangleHeight: waveform.height
                property real triangleWidth: 0.25 * waveform.effectiveZoomFactor
                property int numTriangles: Math.ceil(width / triangleWidth)

                anchors.top: waveform.top
                anchors.left: waveform.right
                width: waveformContainer.width / 2
                height: waveform.height

                ShapePath {
                    strokeColor: Theme.waveformPostrollColor
                    strokeWidth: 1
                    fillColor: "transparent"

                    PathMultiline {
                        paths: {
                            let p = [];
                            for (let i = 0; i < postroll.numTriangles; i++) {
                                p.push([
                                        Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2),
                                        Qt.point((i + 1) * postroll.triangleWidth, 0),
                                        Qt.point((i + 1) * postroll.triangleWidth, postroll.triangleHeight),
                                        Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2),
                                ]);
                            }
                            return p;
                        }
                    }
                }
            }

            Repeater {
                model: root.deckPlayer.beatsModel

                Rectangle {
                    property real alpha: 0.9 // TODO: Make this configurable (i.e., "[Waveform],beatGridAlpha" config option)

                    width: 1
                    height: waveform.height
                    x: (framePosition * 2 / samplesControl.value) * waveform.width
                    color: Theme.waveformBeatColor
                }
            }

            Skin.WaveformIntroOutro {
                id: intro

                visible: introStartPosition.value != -1 || introEndPosition.value != -1

                height: waveform.height
                x: ((introStartPosition.value != -1 ? introStartPosition.value : introEndPosition.value) / samplesControl.value) * waveform.width
                width: introEndPosition.value == -1 ? 0 : ((introEndPosition.value - introStartPosition.value) / samplesControl.value) * waveform.width
            }

            Skin.WaveformIntroOutro {
                id: outro

                visible: outroStartPosition.value != -1 || outroEndPosition.value != -1
                isIntro: false

                height: waveform.height
                x: ((outroStartPosition.value != -1 ? outroStartPosition.value : outroEndPosition.value) / samplesControl.value) * waveform.width
                width: outroEndPosition.value == -1 || outroStartPosition.value == -1 ? 0 : ((outroEndPosition.value - outroStartPosition.value) / samplesControl.value) * waveform.width
            }

            Skin.WaveformLoop {
                id: loop

                visible: loopStartPosition.value != -1 && loopEndPosition.value != -1

                height: waveform.height
                x: (loopStartPosition.value / samplesControl.value) * waveform.width
                width: ((loopEndPosition.value - loopStartPosition.value) / samplesControl.value) * waveform.width
                enabled: loopEnabled.value
            }

            Repeater {
                model: root.deckPlayer.hotcuesModel

                Item {
                    id: cue

                    required property int startPosition
                    required property int endPosition
                    required property string label
                    required property bool isLoop
                    required property int hotcueNumber

                    Skin.WaveformHotcue {
                        group: root.group
                        hotcueNumber: cue.hotcueNumber + 1
                        label: cue.label
                        isLoop: cue.isLoop

                        x: (startPosition * 2 / samplesControl.value) * waveform.width
                        width: cue.isLoop ? ((endPosition - startPosition) * 2 / samplesControl.value) * waveform.width : null
                        height: waveform.height
                    }
                }
            }

            Skin.WaveformCue {
                id: maincue

                height: waveform.height
                x: (mainCuePosition.value / samplesControl.value) * waveform.width
            }
        }
    }

    Shape {
        id: playMarkerShape

        anchors.fill: parent

        ShapePath {
            id: playMarker

            property real screenPosition: 0.5

            startX: playMarkerShape.width * playMarker.screenPosition
            startY: 0
            strokeColor: Theme.waveformCursorColor
            strokeWidth: 1

            PathLine {
                id: marker

                x: playMarkerShape.width * playMarker.screenPosition
                y: playMarkerShape.height
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

    MouseArea {
        property int mouseStatus: WaveformRow.MouseStatus.Normal
        property point mouseAnchor: Qt.point(0, 0)

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            mouseAnchor = Qt.point(mouse.x, mouse.y);
            if (mouse.button == Qt.LeftButton) {
                if (mouseStatus == WaveformRow.MouseStatus.Bending)
                    wheelControl.parameter = 0.5;

                mouseStatus = WaveformRow.MouseStatus.Scratching;
                scratchPositionEnableControl.value = 1;
                // TODO: Calculate position properly
                scratchPositionControl.value = -mouse.x * waveform.effectiveZoomFactor * 2;
                console.log(mouse.x);
            } else {
                if (mouseStatus == WaveformRow.MouseStatus.Scratching)
                    scratchPositionEnableControl.value = 0;

                wheelControl.parameter = 0.5;
                mouseStatus = WaveformRow.MouseStatus.Bending;
            }
        }
        onPositionChanged: {
            switch (mouseStatus) {
                case WaveformRow.MouseStatus.Bending: {
                    const diff = mouse.x - mouseAnchor.x;
                    // Start at the middle of [0.0, 1.0], and emit values based on how far
                    // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
                    // this is tuned to 127.
                    const v = 0.5 + (diff / 1270);
                    // clamp to [0.0, 1.0]
                    wheelControl.parameter = Mixxx.MathUtils.clamp(v, 0, 1);
                    break;
                };
                case WaveformRow.MouseStatus.Scratching:
                // TODO: Calculate position properly
                    scratchPositionControl.value = -mouse.x * waveform.effectiveZoomFactor * 2;
                    break;
            }
        }
        onReleased: {
            switch (mouseStatus) {
                case WaveformRow.MouseStatus.Bending:
                    wheelControl.parameter = 0.5;
                    break;
                case WaveformRow.MouseStatus.Scratching:
                    scratchPositionEnableControl.value = 0;
                    break;
            }
            mouseStatus = WaveformRow.MouseStatus.Normal;
        }
    }
}
