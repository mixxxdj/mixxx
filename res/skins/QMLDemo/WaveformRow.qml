import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    required property string group

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

        Mixxx.WaveformOverview {
            id: waveform

            width: waveformContainer.duration * 10 * rateRatioControl.value * zoomControl.value
            height: parent.height
            x: 0.5 * waveformContainer.width - playPositionControl.value * waveform.width
            player: Mixxx.PlayerManager.getPlayer(root.group)
        }

    }

    Shape {
        id: playMarkerShape

        anchors.fill: parent

        ShapePath {
            id: playMarkerPath

            startX: parent.width / 2
            startY: 0
            strokeColor: "white"
            strokeWidth: 1

            PathLine {
                id: marker

                x: playMarkerPath.startX
                y: playMarkerShape.height
            }

        }

    }

}
