import Mixxx 1.0 as Mixxx
import QtQuick 2.12

ShaderEffect {
    id: root

    property string group // required
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property size framebufferSize: Qt.size(width, height)
    property int waveformLength: root.deckPlayer.waveformLength
    property int textureSize: root.deckPlayer.waveformTextureSize
    property int textureStride: root.deckPlayer.waveformTextureStride
    property real firstVisualIndex: 1
    property real lastVisualIndex: root.deckPlayer.waveformLength / 2
    property color axesColor: "#FFFFFF"
    property color highColor: "#0000FF"
    property color midColor: "#00FF00"
    property color lowColor: "#FF0000"
    property real highGain: filterWaveformEnableControl.value ? (filterHighKillControl.value ? 0 : filterHighControl.value) : 1
    property real midGain: filterWaveformEnableControl.value ? (filterMidKillControl.value ? 0 : filterMidControl.value) : 1
    property real lowGain: filterWaveformEnableControl.value ? (filterLowKillControl.value ? 0 : filterLowControl.value) : 1
    property real allGain: pregainControl.value
    property Image waveformTexture

    fragmentShader: "qrc:/shaders/rgbsignal_qml.frag.qsb"

    Mixxx.ControlProxy {
        id: pregainControl

        group: root.group
        key: "pregain"
    }

    Mixxx.ControlProxy {
        id: filterWaveformEnableControl

        group: root.group
        key: "filterWaveformEnable"
    }

    Mixxx.ControlProxy {
        id: filterHighControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter3"
    }

    Mixxx.ControlProxy {
        id: filterHighKillControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "button_parameter3"
    }

    Mixxx.ControlProxy {
        id: filterMidControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter2"
    }

    Mixxx.ControlProxy {
        id: filterMidKillControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "button_parameter2"
    }

    Mixxx.ControlProxy {
        id: filterLowControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter1"
    }

    Mixxx.ControlProxy {
        id: filterLowKillControl

        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "button_parameter1"
    }

    waveformTexture: Image {
        visible: false
        layer.enabled: false
        source: root.deckPlayer.waveformTexture
    }
}
