import QtQuick 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Mixxx.ControllerScreen {
    id: root

    required property string screenId

    Mixxx.ControlProxy {
        id: tempoEncoderTouch
        group: "[Controls]"
        key: "touch_shift"
    }

    Component {
        id: deckWaveformComponent

        Item {
            id: deck

            property string group: parent.deckGroup
            property string deckName: parent.deckLabel
            property color accentColor: parent.deckAccent

            property var player: Mixxx.PlayerManager.getPlayer(group)

        Mixxx.ControlProxy {
            id: trackLoaded
            group: deck.group
            key: "track_loaded"

            onValueChanged: {
                deck.player = Mixxx.PlayerManager.getPlayer(deck.group);
            }
        }

        Mixxx.ControlProxy {
            id: waveformZoom
            group: deck.group
            key: "waveform_zoom"
        }

        Mixxx.ControlProxy {
            id: bpm
            group: deck.group
            key: "bpm"
        }

        Rectangle {
            anchors.fill: parent
            color: "#05070a"
        }

        MixxxControls.WaveformDisplay {
            anchors.fill: parent
            group: deck.group
            // Show roughly 17 beats where the standard waveform zoom shows 9.
            zoom: Math.min(10, Math.max(1, waveformZoom.value * 1.9))
            backgroundColor: "#05070a"

            Mixxx.WaveformRendererEndOfTrack {
                color: "#802020"
                endOfTrackWarningTime: 30
            }

            Mixxx.WaveformRendererPreroll {
                color: "#554433"
            }

            Mixxx.WaveformRendererMarkRange {
                Mixxx.WaveformMarkRange {
                    startControl: "loop_start_position"
                    endControl: "loop_end_position"
                    enabledControl: "loop_enabled"
                    color: deck.accentColor
                    opacity: 0.32
                    disabledColor: "#808080"
                    disabledOpacity: 0.18
                }
            }

            Mixxx.WaveformRendererRGB {
                axesColor: "#60ffffff"
                lowColor: deck.deckName === "DECK 1" ? "#e84a5f" : "#36b9e8"
                midColor: deck.deckName === "DECK 1" ? "#ffd166" : "#63e6be"
                highColor: "#f5f7fa"
                gainAll: 1.0
                gainLow: 1.0
                gainMid: 1.0
                gainHigh: 1.0
            }

            Mixxx.WaveformRendererBeat {
                color: "#70ffffff"
            }

            Mixxx.WaveformRendererMark {
                playMarkerColor: deck.accentColor
                playMarkerBackground: "#50000000"
                defaultMark: Mixxx.WaveformMark {
                    align: "bottom|center"
                    color: deck.accentColor
                    textColor: "#ffffff"
                    text: " %1 "
                }

                untilMark.showTime: false
                untilMark.showBeats: true
                untilMark.align: Qt.AlignCenter
                untilMark.textSize: 12
            }
        }

        Rectangle {
            x: parent.width / 2
            width: 2
            height: parent.height
            color: deck.accentColor
            opacity: 0.9
        }

        Rectangle {
            x: 5
            y: 4
            width: labelRow.width + 10
            height: 22
            radius: 3
            color: "#b0000000"

            Row {
                id: labelRow
                anchors.centerIn: parent
                spacing: 8

                Text {
                    text: deck.deckName
                    color: deck.accentColor
                    font.family: "Noto Sans"
                    font.pixelSize: 13
                    font.bold: true
                }

                Text {
                    text: deck.player && deck.player.isLoaded ? (deck.player.artist.length > 0 ? deck.player.artist + " — " + deck.player.title : deck.player.title) : "NO TRACK"
                    color: "#f2f2f2"
                    font.family: "Noto Sans"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    width: Math.min(520, implicitWidth)
                }
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: 5
            y: 4
            width: bpmText.width + 10
            height: 22
            radius: 3
            color: "#b0000000"

            Text {
                id: bpmText
                anchors.centerIn: parent
                text: bpm.value > 0 ? Number(bpm.value).toFixed(1) + " BPM" : "--.- BPM"
                color: "#f2f2f2"
                font.family: "Noto Sans"
                font.pixelSize: 12
                font.bold: true
            }
        }
        }
    }

    init: function (controllerName, isDebug) {
        console.log("Starting Push 2 dual-deck waveform screen for " + controllerName);
    }

    shutdown: function () {
        console.log("Stopping Push 2 dual-deck waveform screen");
    }

    transformFrame: function (input, timestamp) {
        // The patched C++ USB backend performs the Push-specific XOR and
        // line padding without a per-byte JavaScript loop.
        return input;
    }

    Rectangle {
        id: waveformView
        anchors.fill: parent
        color: "black"
        visible: tempoEncoderTouch.value <= 0

        Loader {
            id: deck1
            property string deckGroup: "[Channel1]"
            property string deckLabel: "DECK 1"
            property color deckAccent: "#ff6b6b"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 79
            sourceComponent: deckWaveformComponent
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            y: 79
            height: 2
            color: "#d0d5db"
        }

        Loader {
            id: deck2
            property string deckGroup: "[Channel2]"
            property string deckLabel: "DECK 2"
            property color deckAccent: "#4dabf7"

            anchors.left: parent.left
            anchors.right: parent.right
            y: 81
            height: 79
            sourceComponent: deckWaveformComponent
        }
    }

    Rectangle {
        id: libraryView
        anchors.fill: parent
        color: "#05070a"
        visible: tempoEncoderTouch.value > 0
        z: 10

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 28
            color: "#18202a"

            Text {
                anchors.centerIn: parent
                text: "LIBRARY  ·  TEMPO: BROWSE  ·  \u2190 DECK 1  ·  DECK 2 \u2192"
                color: "#f2f2f2"
                font.family: "Noto Sans"
                font.pixelSize: 13
                font.bold: true
            }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 30
            anchors.bottom: parent.bottom

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24
                y: 12
                text: Mixxx.Library.selectedTitle.length > 0
                        ? Mixxx.Library.selectedTitle
                        : "Turn Tempo to select a track"
                color: "#ffffff"
                font.family: "Noto Sans"
                font.pixelSize: 28
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24
                y: 52
                text: Mixxx.Library.selectedArtist
                color: "#62c5ff"
                font.family: "Noto Sans"
                font.pixelSize: 20
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 24
                y: 84
                text: Mixxx.Library.selectedAlbum
                color: "#aeb8c3"
                font.family: "Noto Sans"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
        }
    }
}
