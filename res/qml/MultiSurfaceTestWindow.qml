import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "Theme"
import "." as Skin

Window {
    id: root

    readonly property var targetScreen: {
        const screens = Qt.application.screens;

        for (let i = 0; i < screens.length; ++i) {
            if (screens[i].width === 800 && screens[i].height === 480)
                return screens[i];
        }

        return screens[0];
    }

    screen: targetScreen
    x: targetScreen.virtualX
    y: targetScreen.virtualY

    width: 800
    height: 480
    visibility: Window.FullScreen
    title: "Mixxx Multi-Surface PoC"
    color: Theme.backgroundColor

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: Theme.embeddedBackgroundColor

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
                    text: "Deck 1"
                    color: Theme.textColor
                    font.pixelSize: 24
                    font.bold: true
                }

                Label {
                    text: "BPM: " + Number(deck1Bpm.value).toFixed(2)
                    color: Theme.textColor
                    font.pixelSize: 20
                }

                Skin.WaveformDisplay {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    group: "[Channel1]"
                }

                Label {
                    text: deck1Play.value > 0 ? "PLAYING" : "STOPPED"
                    color: Theme.textColor
                    font.pixelSize: 18
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: Theme.embeddedBackgroundColor

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
                    text: "Deck 2"
                    color: Theme.textColor
                    font.pixelSize: 24
                    font.bold: true
                }

                Label {
                    text: "BPM: " + Number(deck2Bpm.value).toFixed(2)
                    color: Theme.textColor
                    font.pixelSize: 20
                }

                Skin.WaveformDisplay {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    group: "[Channel2]"
                }

                Label {
                    text: deck2Play.value > 0 ? "PLAYING" : "STOPPED"
                    color: Theme.textColor
                    font.pixelSize: 18
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
