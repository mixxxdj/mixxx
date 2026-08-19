import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme"

Item {
    id: root

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: Theme.embeddedBackgroundColor
            radius: 8

            Mixxx.ControlProxy {
                id: deck1Play

                group: "[Channel1]"
                key: "play"
            }
            Mixxx.ControlProxy {
                id: deck1Bpm

                group: "[Channel1]"
                key: "bpm"
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Label {
                    color: Theme.textColor
                    font.bold: true
                    font.pixelSize: 24
                    text: "Deck 1"
                }
                Label {
                    color: Theme.textColor
                    font.pixelSize: 20
                    text: "BPM: " + Number(deck1Bpm.value).toFixed(2)
                }
                Skin.WaveformDisplay {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    group: "[Channel1]"
                }
                Label {
                    color: Theme.textColor
                    font.pixelSize: 18
                    text: deck1Play.value > 0 ? "PLAYING" : "STOPPED"
                }
                Item {
                    Layout.fillHeight: true
                }
                Button {
                    Layout.fillWidth: true
                    text: deck1Play.value > 0 ? "Pause" : "Play"

                    onClicked: {
                        deck1Play.value = deck1Play.value > 0 ? 0 : 1;
                    }
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: Theme.embeddedBackgroundColor
            radius: 8

            Mixxx.ControlProxy {
                id: deck2Play

                group: "[Channel2]"
                key: "play"
            }
            Mixxx.ControlProxy {
                id: deck2Bpm

                group: "[Channel2]"
                key: "bpm"
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Label {
                    color: Theme.textColor
                    font.bold: true
                    font.pixelSize: 24
                    text: "Deck 2"
                }
                Label {
                    color: Theme.textColor
                    font.pixelSize: 20
                    text: "BPM: " + Number(deck2Bpm.value).toFixed(2)
                }
                Skin.WaveformDisplay {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    group: "[Channel2]"
                }
                Label {
                    color: Theme.textColor
                    font.pixelSize: 18
                    text: deck2Play.value > 0 ? "PLAYING" : "STOPPED"
                }
                Item {
                    Layout.fillHeight: true
                }
                Button {
                    Layout.fillWidth: true
                    text: deck2Play.value > 0 ? "Pause" : "Play"

                    onClicked: {
                        deck2Play.value = deck2Play.value > 0 ? 0 : 1;
                    }
                }
            }
        }
    }
}
