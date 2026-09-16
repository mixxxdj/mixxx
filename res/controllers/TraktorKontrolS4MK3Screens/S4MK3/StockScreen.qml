import QtQuick 2.15
import QtQuick.Layouts 1.3

import "../../../qml" as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import S4MK3 as S4MK3

Rectangle {
    id: root

    required property string group
    required property string screenId

    anchors.fill: parent
    color: "black"

    function onSharedDataUpdate(data) {
        if (!root) return;

        console.log(`Received data on screen#${root.screenId} while currently bind to ${root.group}: ${JSON.stringify(data)}`);
        if (typeof data === "object" && typeof data.group[root.screenId] === "string" && root.group !== data.group[root.screenId]) {
            root.group = data.group[root.screenId]
            waveformOverview.player = Mixxx.PlayerManager.getPlayer(root.group)
            artwork.player = Mixxx.PlayerManager.getPlayer(root.group)
            console.log(`Changed group for screen ${root.screenId} to ${root.group}`);
        }
        var shouldBeCompacted = false;
        if (typeof data.padsMode === "object") {
            scrollingWaveform.visible = data.padsMode[root.group] === 4
            artworkSpacer.visible = data.padsMode[root.group] === 1
            shouldBeCompacted |= scrollingWaveform.visible || artworkSpacer.visible
        }
        if (typeof data.keyboardMode === "object") {
            shouldBeCompacted |= data.keyboardMode[root.group]
            keyboard.visible = !!data.keyboardMode[root.group]
        }
        deckInfo.state = shouldBeCompacted ? "compacted" : ""
        if (typeof data.displayBeatloopSize === "object") {
            timeIndicator.mode = data.displayBeatloopSize[root.group] ? S4MK3.TimeAndBeatloopIndicator.Mode.BeetjumpSize : S4MK3.TimeAndBeatloopIndicator.Mode.RemainingTime
            timeIndicator.update()
        }
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"

        onValueChanged: (value) => {
            if (!value && deckInfo) {
                deckInfo.state = ""
                scrollingWaveform.visible = false
            }
        }
    }

    Timer {
        id: channelchange

        interval: 5000
        repeat: true
        running: false

        onTriggered: {
            root.onSharedDataUpdate({
                    group: {
                        "leftdeck": screenId === "leftdeck" && trackLoadedControl.group === "[Channel1]" ? "[Channel3]" : "[Channel1]",
                        "rightdeck": screenId === "rightdeck" && trackLoadedControl.group === "[Channel2]" ? "[Channel4]" : "[Channel2]",
                    },
                    scrollingWaveform: {
                        "[Channel1]": true,
                        "[Channel2]": true,
                        "[Channel3]": true,
                        "[Channel4]": true,
                    },
                    keyboardMode: {
                        "[Channel1]": false,
                        "[Channel2]": false,
                        "[Channel3]": false,
                        "[Channel4]": false,
                    },
                    displayBeatloopSize: {
                        "[Channel1]": false,
                        "[Channel2]": false,
                        "[Channel3]": false,
                        "[Channel4]": false,
                    },
            });
        }
    }

    Component.onCompleted: {
        if (typeof engine.makeSharedDataConnection !== "function") {
            return
        }

        engine.makeSharedDataConnection(root.onSharedDataUpdate)

        root.onSharedDataUpdate({
                group: {
                    "leftdeck": "[Channel1]",
                    "rightdeck": "[Channel2]",
                },
                scrollingWaveform: {
                    "[Channel1]": false,
                    "[Channel2]": false,
                    "[Channel3]": false,
                    "[Channel4]": false,
                },
                keyboardMode: {
                    "[Channel1]": false,
                    "[Channel2]": false,
                    "[Channel3]": false,
                    "[Channel4]": false,
                },
                displayBeatloopSize: {
                    "[Channel1]": false,
                    "[Channel2]": false,
                    "[Channel3]": false,
                    "[Channel4]": false,
                },
        });
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        Image {
            id: artwork
            anchors.fill: parent

            property var player: Mixxx.PlayerManager.getPlayer(root.group)

            source: player.currentTrack?.coverArtUrl
            height: 100
            width: 100
            fillMode: Image.PreserveAspectFit

            opacity: artworkSpacer.visible ? 1 : 0.2
            z: -1
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 1

                S4MK3.OnAirTrack {
                    id: onAir
                    group: root.group
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    scrolling: !scrollingWaveform.visible
                }
            }
        }

        // Indicator
        Rectangle {
            id: deckInfo

            Layout.fillWidth: true
            Layout.preferredHeight: 105
            Layout.leftMargin: 6
            Layout.rightMargin: 6
            color: "transparent"

            GridLayout {
                id: gridLayout
                anchors.fill: parent
                columnSpacing: 6
                rowSpacing: 6
                columns: 2

                // Section: Key
                S4MK3.KeyIndicator {
                    id: keyIndicator
                    group: root.group
                    borderColor: smallBoxBorder

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Section: Bpm
                S4MK3.BPMIndicator {
                    id: bpmIndicator
                    group: root.group
                    borderColor: smallBoxBorder

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Section: Key
                S4MK3.TimeAndBeatloopIndicator {
                    id: timeIndicator
                    group: root.group

                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    timeColor: smallBoxBorder
                }

                // Section: Bpm
                S4MK3.LoopSizeIndicator {
                    id: loopSizeIndicator
                    group: root.group

                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                }
            }
            states: State {
                name: "compacted"

                PropertyChanges {
                    target:deckInfo
                    Layout.preferredHeight: 28
                }
                PropertyChanges {
                    target: gridLayout
                    columns: 4
                }
                PropertyChanges {
                    target: bpmIndicator
                    state: "compacted"
                }
                PropertyChanges {
                    target: timeIndicator
                    Layout.preferredHeight: -1
                    Layout.fillHeight: true
                    state: "compacted"
                }
                PropertyChanges {
                    target: loopSizeIndicator
                    Layout.preferredHeight: -1
                    Layout.fillHeight: true
                    state: "compacted"
                }
            }
        }

        Item {
            id: scrollingWaveform

            Layout.fillWidth: true
            Layout.minimumHeight: scrollingWaveform.visible ? 120 : 0
            Layout.leftMargin: 0
            Layout.rightMargin: 0

            visible: false

            Mixxx.ControlProxy {
                id: zoomControl

                group: root.group
                key: "waveform_zoom"
            }

            MixxxControls.WaveformDisplay {
                id: singleWaveform
                group: root.group
                x: 0
                width: 320
                height: 100

                Behavior on height { PropertyAnimation { duration: 90} }
                anchors.fill: parent
                zoom: zoomControl.value
                backgroundColor: "#36000000"

                Mixxx.WaveformRendererEndOfTrack {
                    color: 'blue'
                    endOfTrackWarningTime: 30
                }

                Mixxx.WaveformRendererPreroll {
                    color: '#998977'
                }

                Mixxx.WaveformRendererMarkRange {
                    // <!-- Loop -->
                    Mixxx.WaveformMarkRange {
                        startControl: "loop_start_position"
                        endControl: "loop_end_position"
                        enabledControl: "loop_enabled"
                        color: '#00b400'
                        opacity: 0.7
                        disabledColor: '#FFFFFF'
                        disabledOpacity: 0.6
                    }
                    // <!-- Intro -->
                    Mixxx.WaveformMarkRange {
                        startControl: "intro_start_position"
                        endControl: "intro_end_position"
                        color: '#2c5c9a'
                        opacity: 0.6
                        durationTextColor: '#ffffff'
                        durationTextLocation: 'after'
                    }
                    // <!-- Outro -->
                    Mixxx.WaveformMarkRange {
                        startControl: "outro_start_position"
                        endControl: "outro_end_position"
                        color: '#2c5c9a'
                        opacity: 0.6
                        durationTextColor: '#ffffff'
                        durationTextLocation: 'before'
                    }
                }

                Mixxx.WaveformRendererRGB {
                    axesColor: '#00ffffff'
                    lowColor: 'red'
                    midColor: 'green'
                    highColor: 'blue'

                    gainAll: 1.0
                    gainLow: 1.0
                    gainMid: 1.0
                    gainHigh: 1.0
                }

                Mixxx.WaveformRendererStem {
                    gainAll: 1.0
                }

                Mixxx.WaveformRendererBeat {
                    color: '#cfcfcf'
                }

                Mixxx.WaveformRendererMark {
                    playMarkerColor: 'cyan'
                    playMarkerBackground: 'transparent'
                    defaultMark: Mixxx.WaveformMark {
                        align: "bottom|right"
                        color: "#FF0000"
                        textColor: "#FFFFFF"
                        text: " %1 "
                    }

                    untilMark.showTime: true
                    untilMark.showBeats: true
                    untilMark.align: Qt.AlignCenter
                    untilMark.textSize: 14

                    Mixxx.WaveformMark {
                        control: "cue_point"
                        text: 'C'
                        align: 'top|right'
                        color: 'red'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "loop_start_position"
                        text: '↻'
                        align: 'top|left'
                        color: 'green'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "loop_end_position"
                        align: 'bottom|right'
                        color: 'green'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "intro_start_position"
                        align: 'top|right'
                        color: 'blue'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "intro_end_position"
                        text: '◢'
                        align: 'top|left'
                        color: 'blue'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "outro_start_position"
                        text: '◣'
                        align: 'top|right'
                        color: 'blue'
                        textColor: '#FFFFFF'
                    }
                    Mixxx.WaveformMark {
                        control: "outro_end_position"
                        align: 'top|left'
                        color: 'blue'
                        textColor: '#FFFFFF'
                    }
                }
            }
        }

        Mixxx.ControlProxy {
            id: deckScratching

            group: root.group
            key: "scratch2_enable"

            onValueChanged: {
                if (typeof engine.makeSharedDataConnection !== "function") {
                    return;
                }

                if (value) {
                    waveformTimer.running = false;
                    scrollingWaveform.visible = true;
                    deckInfo.state = scrollingWaveform.visible ? "compacted" : ""
                } else {
                    waveformTimer.running = true;
                    waveformTimer.restart()
                }
            }
        }

        Timer {
            id: waveformTimer

            interval: 4000
            repeat: false
            running: false

            onTriggered: {
                scrollingWaveform.visible = false;
                deckInfo.state = scrollingWaveform.visible ? "compacted" : ""
            }
        }

        // Spacer
        Item {
            id: artworkSpacer

            Layout.fillWidth: true
            Layout.minimumHeight: artworkSpacer.visible ? 120 : 0
            Layout.leftMargin: 6
            Layout.rightMargin: 6

            visible: false

            Rectangle {
                color: "transparent"
                visible: parent.visible
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                x: 153
                width: 2
            }
        }

        // Track progress
        Item {
            id: waveform
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 6
            Layout.rightMargin: 6
            layer.enabled: true

            S4MK3.Progression {
                id: progression
                group: root.group

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
            }

            Mixxx.WaveformOverview {
                readonly property var player: Mixxx.PlayerManager.getPlayer(root.group)
                id: waveformOverview
                anchors.fill: parent

                track: player.currentTrack
            }

            Mixxx.ControlProxy {
                id: samplesControl

                group: root.group
                key: "track_samples"
            }

            // Hotcue
            Repeater {
                model: 16

                S4MK3.HotcuePoint {
                    required property int index

                    Mixxx.ControlProxy {
                        id: samplesControl

                        group: root.group
                        key: "track_samples"
                    }

                    Mixxx.ControlProxy {
                        id: hotcueEnabled
                        group: root.group
                        key: `hotcue_${index + 1}_status`
                    }

                    Mixxx.ControlProxy {
                        id: hotcuePosition
                        group: root.group
                        key: `hotcue_${index + 1}_position`
                    }

                    Mixxx.ControlProxy {
                        id: hotcueColor
                        group: root.group
                        key: `hotcue_${number}_color`
                    }

                    anchors.top: parent.top
                    // anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    visible: hotcueEnabled.value

                    number: this.index + 1
                    type: S4MK3.HotcuePoint.Type.OneShot
                    position: hotcuePosition.value / samplesControl.value
                    color: `#${(hotcueColor.value >> 16).toString(16).padStart(2, '0')}${((hotcueColor.value >> 8) & 255).toString(16).padStart(2, '0')}${(hotcueColor.value & 255).toString(16).padStart(2, '0')}`
                }
            }

            // Intro
            S4MK3.HotcuePoint {

                Mixxx.ControlProxy {
                    id: introStartEnabled
                    group: root.group
                    key: `intro_start_enabled`
                }

                Mixxx.ControlProxy {
                    id: introStartPosition
                    group: root.group
                    key: `intro_start_position`
                }

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: introStartEnabled.value

                type: S4MK3.HotcuePoint.Type.IntroIn
                position: introStartPosition.value / samplesControl.value
            }

            // Extro
            S4MK3.HotcuePoint {

                Mixxx.ControlProxy {
                    id: introEndEnabled
                    group: root.group
                    key: `intro_end_enabled`
                }

                Mixxx.ControlProxy {
                    id: introEndPosition
                    group: root.group
                    key: `intro_end_position`
                }

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: introEndEnabled.value

                type: S4MK3.HotcuePoint.Type.IntroOut
                position: introEndPosition.value / samplesControl.value
            }

            // Loop in
            S4MK3.HotcuePoint {
                Mixxx.ControlProxy {
                    id: loopStartPosition
                    group: root.group
                    key: `loop_start_position`
                }

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: loopStartPosition.value > 0

                type: S4MK3.HotcuePoint.Type.LoopIn
                position: loopStartPosition.value / samplesControl.value
            }

            // Loop out
            S4MK3.HotcuePoint {
                Mixxx.ControlProxy {
                    id: loopEndPosition
                    group: root.group
                    key: `loop_end_position`
                }

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: loopEndPosition.value > 0

                type: S4MK3.HotcuePoint.Type.LoopOut
                position: loopEndPosition.value / samplesControl.value
            }
        }

        S4MK3.Keyboard {
            id: keyboard
            group: root.group
            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 6
            Layout.rightMargin: 6
        }
    }
}
