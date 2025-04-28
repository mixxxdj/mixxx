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

Mixxx.ControllerScreen {
    id: root

    required property string screenId
    property color fontColor: Qt.rgba(242/255,242/255,242/255, 1)
    property color smallBoxBorder: Qt.rgba(44/255,44/255,44/255, 1)

    property string group: "[Channel1]"
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)

    init: function(controlerName, isDebug) {
        console.log(`Screen ${root.screenId} has started`)
        switch (root.screenId) {
            case "jog":
                loader.sourceComponent = jog
                break;
            default:
                loader.sourceComponent = main
        }
    }

    shutdown: function() {
        console.log(`Screen ${root.screenId} is stopping`)
        loader.sourceComponent = splash
    }

    // function transformFrame(input: ArrayBuffer, timestamp: date) {
    transformFrame: function(input, timestamp) {
        return new ArrayBuffer(0);
    }

    Mixxx.ControlProxy {
        group: root.group
        key: "track_loaded"

        onValueChanged: (value) => {
            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group)
        }
    }

    Timer {
        id: channelchange

        interval: 2000
        repeat: true
        running: true

        onTriggered: {
            root.group = root.group === "[Channel1]" ? "[Channel2]" : "[Channel1]"
            deckPlayer = Mixxx.PlayerManager.getPlayer(root.group)
        }
    }

    Component {
        id: splash
        Rectangle {
            color: "black"
            anchors.fill: parent
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
                visible: deckPlayer.trackLocationUrl.toString().length !== 0

                source: deckPlayer.coverArtUrl ?? "../images/templates/logo_mixxx.png"
                height: 100
                width: 100
                fillMode: Image.PreserveAspectFit
            }

            Text {
                visible: deckPlayer.trackLocationUrl.toString().length === 0

                text: qsTr("No Track Loaded")
                font.pixelSize: 12
                font.family: "Noto Sans"
                font.letterSpacing: -1
                color: "white"
            }
        }
    }

    Component {
        id: main

        Rectangle {
            id: debugValue
            anchors.fill: parent
            color: 'black'

            antialiasing: true

            ColumnLayout {
                id: column
                anchors.fill: parent
                anchors.leftMargin: 0
                anchors.rightMargin: 0
                anchors.topMargin: 0
                anchors.bottomMargin: 0
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Repeater {
                        id: debugColor

                        model: [
                                "black",
                                "white",
                                "red",
                                "green",
                                "blue",
                                Qt.rgba(0, 1, 1),
                        ]

                        Rectangle {
                            required property var modelData

                            color: modelData
                            Layout.fillWidth: true
                            height: 80
                        }
                    }
                }

                RowLayout {
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    anchors.topMargin: 6
                    anchors.bottomMargin: 6

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 6

                    Rectangle {
                        color: 'transparent'
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Text {
                            text: qsTr("Group")
                            font.pixelSize: 18
                            font.family: "Noto Sans"
                            font.letterSpacing: -1
                            color: fontColor
                        }
                    }

                    Rectangle {
                        color: 'transparent'
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Text {
                            text: `${root.group}`
                            font.pixelSize: 18
                            font.family: "Noto Sans"
                            font.letterSpacing: -1
                            color: fontColor
                        }
                    }
                }

                RowLayout {
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    anchors.topMargin: 6
                    anchors.bottomMargin: 6

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 6

                    Rectangle {
                        color: 'transparent'
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Text {
                            text: qsTr("Widget")
                            font.pixelSize: 18
                            font.family: "Noto Sans"
                            font.letterSpacing: -1
                            color: fontColor
                        }
                    }

                    Rectangle {
                        color: 'transparent'
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Skin.HotcueButton {
                            anchors.fill: parent

                            hotcueNumber: 1
                            group: root.group
                        }
                    }
                }

                Repeater {
                    model: [{
                            controllerKey: "beatloop_size",
                            title: "Beatloop Size"
                        }, {
                            controllerKey: "track_samples",
                            title: "Track sample"
                        }, {
                            controllerKey: "track_samplerate",
                            title: "Track sample rate"
                        }, {
                            controllerKey: "playposition",
                            title: "Play position"
                        }, {
                            controllerKey: "rate_ratio",
                            title: "Rate ratio"
                        }, {
                            controllerKey: "waveform_zoom",
                            title: "Waveform zoom"
                        }
                    ]

                    RowLayout {
                        id: row
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        anchors.topMargin: 6
                        anchors.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 4
                        required property var modelData

                        Mixxx.ControlProxy {
                            id: mixxxValue
                            group: root.group
                            key: modelData.controllerKey
                        }

                        Rectangle {
                            color: 'transparent'
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Text {
                                text: qsTr(modelData.title)
                                font.pixelSize: 18
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                color: fontColor
                            }
                        }

                        Rectangle {
                            color: 'transparent'
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Text {
                                text: `${mixxxValue.value}`
                                font.pixelSize: 18
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                color: fontColor
                            }
                        }
                    }
                }
                RowLayout {
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    anchors.topMargin: 6
                    anchors.bottomMargin: 6

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Mixxx.ControlProxy {
                        id: zoomControl

                        group: root.group
                        key: "waveform_zoom"
                    }

                    MixxxControls.WaveformDisplay {
                        group: root.group
                        x: 0
                        width: root.width
                        height: 100

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
                            // Loop
                            Mixxx.WaveformMarkRange {
                                startControl: "loop_start_position"
                                endControl: "loop_end_position"
                                enabledControl: "loop_enabled"
                                color: '#00b400'
                                opacity: 0.7
                                disabledColor: '#FFFFFF'
                                disabledOpacity: 0.6
                            }
                            // Intro
                            Mixxx.WaveformMarkRange {
                                startControl: "intro_start_position"
                                endControl: "intro_end_position"
                                color: '#2c5c9a'
                                opacity: 0.6
                                durationTextColor: '#ffffff'
                                durationTextLocation: 'after'
                            }
                            // Outro
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
                                align: "bottom|center"
                                color: "#FF0000"
                                textColor: "#FFFFFF"
                                text: " %1 "
                            }

                            untilMark.showTime: false
                            untilMark.showBeats: false
                            untilMark.align: Qt.AlignCenter
                            untilMark.textSize: 14

                            Mixxx.WaveformMark {
                                control: "cue_point"
                                text: 'CUE'
                                align: 'top|right'
                                color: '#FF8000'
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
                                text: '◢'
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
                                text: '◣'
                                align: 'top|left'
                                color: 'blue'
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
