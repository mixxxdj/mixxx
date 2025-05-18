import QtQuick 2.15
import QtQuick.Window 2.3

import QtQuick.Controls 2.15
import QtQuick.Shapes 1.11
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import Qt5Compat.GraphicalEffects

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import "." as Skin
import "Theme"

Mixxx.ControllerScreen {
    id: root

    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    property color fontColor: Qt.rgba(242 / 255, 242 / 255, 242 / 255, 1)
    property string group: "[Channel1]"
    required property string screenId
    property color smallBoxBorder: Qt.rgba(44 / 255, 44 / 255, 44 / 255, 1)

    init: function (controlerName, isDebug) {
        console.log(`Screen ${root.screenId} has started`);
        switch (root.screenId) {
        case "jog":
            loader.sourceComponent = jog;
            break;
        default:
            loader.sourceComponent = main;
        }
    }
    shutdown: function () {
        console.log(`Screen ${root.screenId} is stopping`);
        loader.sourceComponent = splash;
    }

    // function transformFrame(input: ArrayBuffer, timestamp: date) {
    transformFrame: function (input, timestamp) {
        return new ArrayBuffer(0);
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "track_loaded"

        onValueChanged: value => {
            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group);
        }
    }
    Timer {
        id: channelchange

        interval: 2000
        repeat: true
        running: true

        onTriggered: {
            root.group = root.group === "[Channel1]" ? "[Channel2]" : "[Channel1]";
            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group);
        }
    }
    Component {
        id: splash

        Rectangle {
            anchors.fill: parent
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: "../images/templates/logo_mixxx.png"
            }
        }
    }
    Component {
        id: jog

        Rectangle {
            anchors.fill: parent
            color: "black"

            Image {
                id: artwork

                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                height: 100
                source: deckPlayer.coverArtUrl ?? "../images/templates/logo_mixxx.png"
                visible: deckPlayer.trackLocationUrl.toString().length !== 0
                width: 100
            }
            Text {
                color: "white"
                font.family: "Noto Sans"
                font.letterSpacing: -1
                font.pixelSize: 12
                text: qsTr("No Track Loaded")
                visible: deckPlayer.trackLocationUrl.toString().length === 0
            }
        }
    }
    Component {
        id: main

        Rectangle {
            id: debugValue

            anchors.fill: parent
            antialiasing: true
            color: 'black'

            GridLayout {
                id: column

                anchors.fill: parent
                columnSpacing: 10
                columns: 2
                rowSpacing: 6

                RowLayout {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    spacing: 0

                    Repeater {
                        id: debugColor

                        model: ["black", "white", "red", "green", "blue", Qt.rgba(0, 1, 1),]

                        Rectangle {
                            required property var modelData

                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: modelData
                        }
                    }
                }
                RowLayout {
                    spacing: 6

                    Text {
                        Layout.fillWidth: true
                        color: fontColor
                        font.family: "Noto Sans"
                        font.letterSpacing: -1
                        font.pixelSize: 18
                        text: qsTr("Group")
                    }
                    Text {
                        Layout.fillWidth: true
                        color: fontColor
                        font.family: "Noto Sans"
                        font.letterSpacing: -1
                        font.pixelSize: 18
                        text: root.group
                    }
                }
                RowLayout {
                    spacing: 6

                    Text {
                        Layout.fillWidth: true
                        color: fontColor
                        font.family: "Noto Sans"
                        font.letterSpacing: -1
                        font.pixelSize: 18
                        text: qsTr("Widget")
                    }
                    Skin.HotcueButton {
                        group: root.group
                        hotcueNumber: 1
                    }
                    Skin.ControlKnob {
                        arcStart: Knob.ArcStart.Minimum
                        color: Theme.gainKnobColor
                        group: root.group
                        height: width
                        key: "pregain"
                        width: 42
                    }
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Repeater {
                        model: [
                            {
                                controllerKey: "beatloop_size",
                                title: "Beatloop Size"
                            },
                            {
                                controllerKey: "track_samples",
                                title: "Track sample"
                            },
                            {
                                controllerKey: "track_samplerate",
                                title: "Track sample rate"
                            },
                            {
                                controllerKey: "playposition",
                                title: "Play position"
                            },
                            {
                                controllerKey: "rate_ratio",
                                title: "Rate ratio"
                            },
                            {
                                controllerKey: "waveform_zoom",
                                title: "Waveform zoom"
                            }
                        ]

                        RowLayout {
                            id: row

                            required property var modelData

                            spacing: 4

                            Mixxx.ControlProxy {
                                id: mixxxValue

                                group: root.group
                                key: modelData.controllerKey
                            }
                            Text {
                                Layout.fillWidth: true
                                color: fontColor
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                font.pixelSize: 18
                                text: qsTr(modelData.title)
                            }
                            Text {
                                color: fontColor
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                font.pixelSize: 18
                                text: mixxxValue.value
                            }
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Repeater {
                        model: ["theme", "idleBackground", "accentColor", "deckA", "intValue",]

                        RowLayout {
                            required property var modelData

                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                color: fontColor
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                font.pixelSize: 18
                                text: modelData
                            }
                            Text {
                                Layout.maximumWidth: 160
                                color: fontColor
                                elide: Text.ElideLeft
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                font.pixelSize: 18
                                text: engine.getSetting(modelData)
                            }
                        }
                    }
                }
                RowLayout {
                    Layout.columnSpan: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Mixxx.ControlProxy {
                        id: zoomControl

                        group: root.group
                        key: "waveform_zoom"
                    }
                    MixxxControls.WaveformDisplay {
                        backgroundColor: "#36000000"
                        group: root.group
                        height: 100
                        width: root.width
                        x: 0
                        zoom: zoomControl.value

                        Mixxx.WaveformRendererEndOfTrack {
                            color: 'blue'
                            endOfTrackWarningTime: 30
                        }
                        Mixxx.WaveformRendererPreroll {
                            color: '#998977'
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
                        Mixxx.WaveformRendererRGB {
                            axesColor: '#00ffffff'
                            gainAll: 1.0
                            gainHigh: 1.0
                            gainLow: 1.0
                            gainMid: 1.0
                            highColor: 'blue'
                            lowColor: 'red'
                            midColor: 'green'
                        }
                        Mixxx.WaveformRendererStem {
                            gainAll: 1.0
                        }
                        Mixxx.WaveformRendererBeat {
                            color: '#cfcfcf'
                        }
                        Mixxx.WaveformRendererMark {
                            playMarkerBackground: 'transparent'
                            playMarkerColor: 'cyan'
                            untilMark.align: Qt.AlignCenter
                            untilMark.showBeats: false
                            untilMark.showTime: false
                            untilMark.textSize: 14

                            defaultMark: Mixxx.WaveformMark {
                                align: "bottom|center"
                                color: "#FF0000"
                                text: " %1 "
                                textColor: "#FFFFFF"
                            }

                            Mixxx.WaveformMark {
                                align: 'top|right'
                                color: '#FF8000'
                                control: "cue_point"
                                text: 'CUE'
                                textColor: '#FFFFFF'
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
                }
            }
        }
    }
    Loader {
        id: loader

        anchors.fill: parent
        sourceComponent: splash
    }
}
