import QtQuick 2.15
import QtQuick.Window 2.3
import QtQuick.Scene3D 2.14

import QtQuick.Controls 2.15
import QtQuick.Shapes 1.11
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import Qt5Compat.GraphicalEffects

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string screenId
    property color fontColor: Qt.rgba(242/255,242/255,242/255, 1)
    property color smallBoxBorder: Qt.rgba(44/255,44/255,44/255, 1)

    property string group: "[Channel1]"
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)

    function init(controlerName, isDebug) {
        console.log(`Screen ${root.screenId} has started`)
        switch (root.screenId) {
            case "jog":
                loader.sourceComponent = jog
                break;
            default:
                loader.sourceComponent = main
        }
    }

    function shutdown() {
        console.log(`Screen ${root.screenId} is stopping`)
        loader.sourceComponent = splash
    }

    // function transformFrame(input: ArrayBuffer, timestamp: date) {
    function transformFrame(input, timestamp) {
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
                            font.pixelSize: 24
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
                            font.pixelSize: 24
                            font.family: "Noto Sans"
                            font.letterSpacing: -1
                            color: fontColor
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
                        spacing: 6
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
                                font.pixelSize: 24
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
                                font.pixelSize: 24
                                font.family: "Noto Sans"
                                font.letterSpacing: -1
                                color: fontColor
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
