import QtQuick 2.15
import QtQuick.Window 2.15

import "." as Skin
import Mixxx 1.0 as Mixxx

Item {
    id: waveform

    property int deckId: deckInfo.deckId

    readonly property string group: `[Channel${deckId}]`

    layer.enabled: true

    Item {
        id: progression

        property real windowWidth: Window.width
        Mixxx.ControlProxy {
            id: propPosition
            group: waveform.group
            key: "playposition"
        }

        Mixxx.ControlProxy {
            id: propVisible
            group: waveform.group
            key: "track_loaded"
        }

        width: propPosition.value * (320 - 12)
        visible: propVisible.value

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        clip: true

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: -border.width
            anchors.topMargin: -border.width
            anchors.bottomMargin: -border.width
            border.width: 2
            border.color:"black"
            color: Qt.rgba(0.39, 0.80, 0.96, 0.3)
        }
    }

    Mixxx.WaveformOverview {
        readonly property var player: Mixxx.PlayerManager.getPlayer(waveform.group)
        id: waveformOverview
        anchors.fill: parent
        anchors.topMargin: 6

        track: player.currentTrack
    }

    Mixxx.ControlProxy {
        id: samplesControl

        group: waveform.group
        key: "track_samples"
    }

    // // Hotcue
    // Repeater {
    //     model: 16

    //     S4MK3.HotcuePoint {
    //         required property int index

    //         Mixxx.ControlProxy {
    //             id: samplesControl

    //             group: waveform.group
    //             key: "track_samples"

    //             onValueChanged: (value) => {
    //                 redraw(waveform)
    //             }
    //         }

    //         Mixxx.ControlProxy {
    //             id: hotcueEnabled
    //             group: waveform.group
    //             key: `hotcue_${index + 1}_status`

    //             onValueChanged: (value) => {
    //                 redraw(waveform)
    //             }
    //         }

    //         Mixxx.ControlProxy {
    //             id: hotcuePosition
    //             group: waveform.group
    //             key: `hotcue_${index + 1}_position`

    //             onValueChanged: (value) => {
    //                 redraw(waveform)
    //             }
    //         }

    //         Mixxx.ControlProxy {
    //             id: hotcueColor
    //             group: waveform.group
    //             key: `hotcue_${number}_color`
    //             onValueChanged: (value) => {
    //                 redraw(waveform)
    //             }
    //         }

    //         anchors.top: parent.top
    //         // anchors.left: parent.left
    //         anchors.bottom: parent.bottom
    //         visible: hotcueEnabled.value

    //         number: this.index + 1
    //         type: S4MK3.HotcuePoint.Type.OneShot
    //         position: hotcuePosition.value / samplesControl.value
    //         color: `#${(hotcueColor.value >> 16).toString(16).padStart(2, '0')}${((hotcueColor.value >> 8) & 255).toString(16).padStart(2, '0')}${(hotcueColor.value & 255).toString(16).padStart(2, '0')}`
    //     }
    // }

    // // Intro
    // S4MK3.HotcuePoint {

    //     Mixxx.ControlProxy {
    //         id: introStartEnabled
    //         group: waveform.group
    //         key: `intro_start_enabled`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     Mixxx.ControlProxy {
    //         id: introStartPosition
    //         group: waveform.group
    //         key: `intro_start_position`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     visible: introStartEnabled.value

    //     type: S4MK3.HotcuePoint.Type.IntroIn
    //     position: introStartPosition.value / samplesControl.value
    // }

    // // Extro
    // S4MK3.HotcuePoint {

    //     Mixxx.ControlProxy {
    //         id: introEndEnabled
    //         group: waveform.group
    //         key: `intro_end_enabled`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     Mixxx.ControlProxy {
    //         id: introEndPosition
    //         group: waveform.group
    //         key: `intro_end_position`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     visible: introEndEnabled.value

    //     type: S4MK3.HotcuePoint.Type.IntroOut
    //     position: introEndPosition.value / samplesControl.value
    // }

    // // Loop in
    // S4MK3.HotcuePoint {
    //     Mixxx.ControlProxy {
    //         id: loopStartPosition
    //         group: waveform.group
    //         key: `loop_start_position`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     visible: loopStartPosition.value > 0

    //     type: S4MK3.HotcuePoint.Type.LoopIn
    //     position: loopStartPosition.value / samplesControl.value
    // }

    // // Loop out
    // S4MK3.HotcuePoint {
    //     Mixxx.ControlProxy {
    //         id: loopEndPosition
    //         group: waveform.group
    //         key: `loop_end_position`

    //         onValueChanged: (value) => {
    //             redraw(waveform)
    //         }
    //     }

    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     visible: loopEndPosition.value > 0

    //     type: S4MK3.HotcuePoint.Type.LoopOut
    //     position: loopEndPosition.value / samplesControl.value
    // }
}
