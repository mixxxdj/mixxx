import Mixxx 1.0 as Mixxx
import QtQuick 2.14
import QtQuick.Shapes 1.12
import "Theme"

Item {
    id: root

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

        Item {
            id: waveform

            property real effectiveZoomFactor: rateRatioControl.value * zoomControl.value * 100

            width: waveformContainer.duration * effectiveZoomFactor
            height: parent.height
            x: 0.5 * waveformContainer.width - playPositionControl.value * width
            visible: root.deckPlayer.isLoaded

            Mixxx.WaveformOverview {
                player: Mixxx.PlayerManager.getPlayer(root.group)
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
                                p.push([Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2), Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, 0), Qt.point(preroll.width - (i + 1) * preroll.triangleWidth, preroll.triangleHeight), Qt.point(preroll.width - i * preroll.triangleWidth, preroll.triangleHeight / 2)]);
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
                                p.push([Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2), Qt.point((i + 1) * postroll.triangleWidth, 0), Qt.point((i + 1) * postroll.triangleWidth, postroll.triangleHeight), Qt.point(i * postroll.triangleWidth, postroll.triangleHeight / 2)]);
                            }
                            return p;
                        }
                    }
                }
            }
        }
    }

    Shape {
        id: playMarkerShape

        anchors.fill: parent

        ShapePath {
            id: playMarkerPath

            startX: parent.width / 2
            startY: 0
            strokeColor: Theme.waveformCursorColor
            strokeWidth: 1

            PathLine {
                id: marker

                x: playMarkerPath.startX
                y: playMarkerShape.height
            }
        }
    }
}
