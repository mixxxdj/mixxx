import QtQuick 2.12
import QtQuick.Layouts 1.12
import Mixxx 1.0 as Mixxx
import "." as Skin
import "Theme"

Item {
    id: root

    implicitHeight: 96

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 6

        // ================= DECK 1 CARD =================
        DeckCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            group: "[Channel1]"
            isDeck1: true
        }

        // ================= DECK 2 CARD =================
        DeckCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            group: "[Channel2]"
            isDeck1: false
        }
    }

    // ================= REUSABLE DECK CARD COMPONENT =================
    component DeckCard: Rectangle {
        id: card

        required property string group
        required property bool isDeck1

        readonly property color accentColor: isDeck1 ? "#00b0ff" : "#ff6d00"
        readonly property color accentDim: isDeck1 ? "#0d47a1" : "#bf360c"
        readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
        readonly property var currentTrack: deckPlayer?.currentTrack
        readonly property bool isLoaded: !!deckPlayer?.isLoaded

        color: "#0f131a"
        radius: 6
        border.color: isDeck1 ? "#00b0ff55" : "#ff6d0055"
        border.width: 1
        clip: true

        // Controls
        Mixxx.ControlProxy { id: bpmControl; group: card.group; key: "bpm" }
        Mixxx.ControlProxy { id: rateControl; group: card.group; key: "rate" }
        Mixxx.ControlProxy { id: durationControl; group: card.group; key: "duration" }
        Mixxx.ControlProxy { id: playPositionControl; group: card.group; key: "playposition" }
        Mixxx.ControlProxy { id: keyControl; group: card.group; key: "visual_key" }
        Mixxx.ControlProxy { id: playControl; group: card.group; key: "play" }
        Mixxx.ControlProxy { id: cueControl; group: card.group; key: "cue_default" }
        Mixxx.ControlProxy { id: syncEnabledControl; group: card.group; key: "sync_enabled" }
        Mixxx.ControlProxy { id: syncModeControl; group: card.group; key: "sync_mode" }

        property int timeDisplayMode: 0 // 0: REMAIN, 1: ELAPSED

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 3

            // -------- 1. TOP HEADER ROW (DECK & TRACK INFO) --------
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                spacing: 6

                // Deck Pill
                Rectangle {
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: 54
                    radius: 3
                    color: card.accentDim
                    border.color: card.accentColor
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: card.isDeck1 ? "DECK 1" : "DECK 2"
                        color: "#ffffff"
                        font.bold: true
                        font.pixelSize: 10
                        font.letterSpacing: 0.5
                    }
                }

                // Track Title & Artist
                Text {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: {
                        if (!card.isLoaded) return "Sin pista - Cargar";
                        var t = card.currentTrack?.title || "";
                        var a = card.currentTrack?.artist || "";
                        return a.length > 0 ? (a + " - " + t) : t;
                    }
                    color: card.isLoaded ? "#ffffff" : "#78909c"
                    font.bold: true
                    font.pixelSize: 11
                }

                // Key Badge
                Rectangle {
                    visible: card.isLoaded && !!keyControl.value && keyText.text.length > 0
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: keyText.implicitWidth + 10
                    radius: 3
                    color: "#182030"
                    border.color: card.accentColor
                    border.width: 1

                    Text {
                        id: keyText
                        anchors.centerIn: parent
                        text: keyControl.value ? keyControl.value.toString() : ""
                        color: "#ffffff"
                        font.bold: true
                        font.pixelSize: 10
                    }
                }
            }

            // -------- 2. MIDDLE DASHBOARD ROW (TIME, BPM, PITCH & TRANSPORT) --------
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                spacing: 6

                // Digital Time Display (Click to toggle Remain / Elapsed)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 4
                    color: "#080b10"
                    border.color: timeMouse.containsMouse ? card.accentColor : "#1e293b"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        spacing: 4

                        Text {
                            text: card.timeDisplayMode === 0 ? "REMAIN" : "TIME"
                            font.bold: true
                            font.pixelSize: 8
                            color: card.accentColor
                        }

                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            font.family: "monospace"
                            font.bold: true
                            font.pixelSize: card.width < 380 ? 13 : 15
                            color: "#ffffff"
                            text: {
                                var total = durationControl.value || 0;
                                var pos = playPositionControl.value || 0;
                                if (total <= 0) return "--:--.--";

                                var cur = Math.max(0, Math.min(total, total * pos));
                                var rem = Math.max(0, total - cur);
                                var target = (card.timeDisplayMode === 0) ? rem : cur;

                                var m = Math.floor(target / 60);
                                var s = Math.floor(target % 60);
                                var ms = Math.floor((target % 1) * 10);
                                return (card.timeDisplayMode === 0 ? "-" : "") +
                                       m + ":" + (s < 10 ? "0" : "") + s + "." + ms;
                            }
                        }
                    }

                    MouseArea {
                        id: timeMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: card.timeDisplayMode = (card.timeDisplayMode === 0 ? 1 : 0)
                    }
                }

                // Pitch / Tempo Percentage
                Rectangle {
                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: card.width < 380 ? 46 : 56
                    radius: 4
                    color: "#080b10"
                    border.color: "#1e293b"
                    border.width: 1

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 0

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "TEMPO"
                            font.pixelSize: 7
                            color: "#90a4ae"
                            font.bold: true
                        }
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            font.bold: true
                            font.pixelSize: 10
                            color: card.accentColor
                            text: {
                                var p = (rateControl.value || 0) * 8.0;
                                return (p >= 0 ? "+" : "") + p.toFixed(1) + "%";
                            }
                        }
                    }
                }

                // BPM Box (Hero display)
                Rectangle {
                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: card.width < 380 ? 70 : 84
                    radius: 4
                    color: "#080b10"
                    border.color: (syncModeControl.value >= 2) ? "#ff9100" : (syncEnabledControl.value > 0 ? "#00e676" : card.accentColor)
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 3
                        spacing: 3

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 0

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "BPM"
                                font.pixelSize: 7
                                color: "#90a4ae"
                                font.bold: true
                            }
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                font.bold: true
                                font.pixelSize: card.width < 380 ? 12 : 14
                                color: (syncModeControl.value >= 2) ? "#ffb74d" : "#ffffff"
                                text: bpmControl.value > 0 ? bpmControl.value.toFixed(1) : "--.-"
                            }
                        }

                        // MASTER / SYNC tag
                        Rectangle {
                            Layout.preferredHeight: 18
                            Layout.preferredWidth: 32
                            radius: 2
                            color: (syncModeControl.value >= 2) ? "#ff9100" : (syncEnabledControl.value > 0 ? "#00e676" : "#1a2230")

                            Text {
                                anchors.centerIn: parent
                                text: (syncModeControl.value >= 2) ? "MSTR" : (syncEnabledControl.value > 0 ? "SYNC" : "SYNC")
                                font.bold: true
                                font.pixelSize: 8
                                color: (syncModeControl.value >= 2 || syncEnabledControl.value > 0) ? "#000000" : "#78909c"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: syncEnabledControl.value = !syncEnabledControl.value
                                onPressAndHold: syncModeControl.value = (syncModeControl.value >= 2 ? 0 : 2)
                            }
                        }
                    }
                }

                // Mini Transport: CUE & PLAY
                Rectangle {
                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: 36
                    radius: 4
                    color: cueMouse.pressed ? "#ff9100" : "#141924"
                    border.color: "#ff9100"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "CUE"
                        font.bold: true
                        font.pixelSize: 10
                        color: cueMouse.pressed ? "#000000" : "#ff9100"
                    }

                    MouseArea {
                        id: cueMouse
                        anchors.fill: parent
                        onPressed: cueControl.value = 1
                        onReleased: cueControl.value = 0
                    }
                }

                Rectangle {
                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: 38
                    radius: 4
                    color: playControl.value > 0 ? "#00e676" : (playMouse.pressed ? "#00b248" : "#141924")
                    border.color: "#00e676"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: playControl.value > 0 ? "❚❚" : "▶"
                        font.bold: true
                        font.pixelSize: 12
                        color: playControl.value > 0 ? "#000000" : "#00e676"
                    }

                    MouseArea {
                        id: playMouse
                        anchors.fill: parent
                        onClicked: playControl.value = (playControl.value > 0 ? 0 : 1)
                    }
                }
            }

            // -------- 3. BOTTOM OVERVIEW WAVEFORM --------
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                radius: 4
                color: "#05070a"
                border.color: "#1e293b"
                border.width: 1
                clip: true

                Skin.WaveformOverview {
                    anchors.fill: parent
                    anchors.margins: 1
                    group: card.group
                }
            }
        }
    }
}
