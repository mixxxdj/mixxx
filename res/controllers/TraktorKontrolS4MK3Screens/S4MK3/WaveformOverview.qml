/*
This module is used to waveform overview, at the bottom of the screen. It is reusing component definition of `WaveformOverview.qml` but remove
the link to markers and provide hooks with screen update/redraw, needed for partial updates.
Currently this section is dedicated to BPM and tempo fader information.
*/
import QtQuick 2.15
import QtQuick.Window 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string group
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    property real scale: 0.2

    visible: false
    antialiasing: true
    anchors.fill: parent

    Connections {
        onGroupChanged: {
            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group)
            console.log("Group changed!!")
        }
    }

    Rectangle {
        color: "white"
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        x: 153
        width: 2
    }
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

        Item {
            id: waveformBeat

            property real effectiveZoomFactor: (zoomControl.value * rateRatioControl.value / root.scale) * 6

            width: waveformContainer.duration * effectiveZoomFactor
            height: parent.height
            x: 0.5 * waveformContainer.width - playPositionControl.value * width
            visible: true

            Shape {
                id: preroll

                property real triangleHeight: waveformBeat.height
                property real triangleWidth: 0.25 * waveformBeat.effectiveZoomFactor
                property int numTriangles: Math.ceil(width / triangleWidth)

                anchors.top: waveformBeat.top
                anchors.right: waveformBeat.left
                width: Math.max(0, waveformBeat.x)
                height: waveformBeat.height

                ShapePath {
                    strokeColor: 'red'
                    strokeWidth: 1
                    fillColor: "transparent"

                    PathMultiline {
                        paths: {
                            let p = [];
                            for (let i = 0; i < preroll.numTriangles; i++) {
                                p.push([Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2), Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, 0), Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, preroll.triangleHeight), Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2)]);
                            }
                            return p;
                        }
                    }
                }
            }

            Shape {
                id: postroll

                property real triangleHeight: waveformBeat.height
                property real triangleWidth: 0.25 * waveformBeat.effectiveZoomFactor
                property int numTriangles: Math.ceil(width / triangleWidth)

                anchors.top: waveformBeat.top
                anchors.left: waveformBeat.right
                width: waveformContainer.width / 2
                height: waveformBeat.height

                ShapePath {
                    strokeColor: 'red'
                    strokeWidth: 1
                    fillColor: "transparent"

                    PathMultiline {
                        paths: {
                            let p = [];
                            for (let i = 0; i < postroll.numTriangles; i++) {
                                p.push([Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2), Qt.point((i + 1) * postroll.triangleWidth, 0), Qt.point((i + 1) * postroll.triangleWidth, postroll.triangleHeight), Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2)]);
                            }
                            return p;
                        }
                    }
                }
            }
        }

        MixxxControls.WaveformOverview {
            id: waveformOverview
            // property real duration: samplesControl.value / sampleRateControl.onValueChanged

            player: root.player
            anchors.fill: parent
            channels: Mixxx.WaveformOverview.Channels.BothChannels
            renderer: Mixxx.WaveformOverview.Renderer.RGB
            colorHigh: 'white'
            colorMid: 'blue'
            colorLow: 'green'
        }
    }
}
