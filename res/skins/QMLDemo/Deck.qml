import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11

Item {
    id: root

    required property string group

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    id: trackInfo

                    Layout.fillWidth: true
                    height: 60
                    color: "#121213"

                    Rectangle {
                        id: coverArt

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        width: height
                        color: "black"
                    }

                    Item {
                        id: spinny

                        anchors.fill: coverArt

                        // The Spinnies are automatically hidden if the track
                        // is stopped. This is not really useful, but is nice for
                        // demo'ing transitions.
                        Mixxx.ControlProxy {
                            group: root.group
                            key: "play"
                            onValueChanged: spinnyIndicator.indicatorVisible = (value > 0)
                        }

                        MixxxControls.Spinny {
                            id: spinnyIndicator

                            anchors.fill: parent
                            group: root.group
                            indicatorVisible: false

                            indicatorDelegate: Image {
                                mipmap: true
                                width: spinnyIndicator.width
                                height: spinnyIndicator.height
                                source: "../LateNight/palemoon/style/spinny_indicator.svg"
                            }

                        }

                    }

                    Item {
                        id: trackText

                        anchors.top: parent.top
                        anchors.left: coverArt.right
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 5

                            Text {
                                id: trackTitle

                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: "Title Placeholder"
                                elide: Text.ElideRight
                                color: "#c2b3a5"
                            }

                            Rectangle {
                                id: trackColor

                                Layout.fillWidth: true
                                implicitHeight: 3
                                color: "black"
                            }

                            Text {
                                id: trackArtist

                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: "Artist Placeholder"
                                elide: Text.ElideRight
                                color: "#c2b3a5"
                            }

                        }

                    }

                }

                WaveformOverview {
                    Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    group: root.group
                }

            }

            Item {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                Layout.fillHeight: true
                implicitWidth: 50

                MixxxControls.ToggleButton {
                    id: syncButton

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24
                    group: root.group
                    key: "sync_enabled"
                    icon.width: 50
                    icon.height: 24

                    State {
                        name: "0"

                        PropertyChanges {
                            target: syncButton
                            icon.source: "../LateNight/palemoon/buttons/btn__sync_deck.svg"
                            icon.color: "#777777"
                            background.color: "transparent"
                            background.border.width: 2
                        }

                        PropertyChanges {
                            target: syncButtonBgImage
                            source: "../LateNight/palemoon/buttons/btn_embedded_sync.svg"
                        }

                    }

                    State {
                        name: "1"

                        PropertyChanges {
                            target: syncButton
                            icon.source: "../LateNight/palemoon/buttons/btn__sync_deck_active.svg"
                            icon.color: "transparent"
                            background.color: "#b24c12"
                            background.border.width: 2
                        }

                        PropertyChanges {
                            target: syncButtonBgImage
                            source: "../LateNight/palemoon/buttons/btn_embedded_sync_active.svg"
                        }

                    }

                    background: Rectangle {
                        anchors.fill: parent

                        data: Image {
                            id: syncButtonBgImage

                            anchors.fill: parent
                        }

                    }

                }

                Mixxx.ControlProxy {
                    id: bpmControl

                    group: root.group
                    key: "bpm"
                }

                Text {
                    id: bpmText

                    anchors.top: syncButton.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: contentHeight
                    text: bpmControl.value.toFixed(2)
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Open Sans"
                    font.bold: true
                    font.pixelSize: 11
                    color: "#777"
                }

                Mixxx.ControlProxy {
                    id: rateRatioControl

                    group: root.group
                    key: "rate_ratio"
                }

                Text {
                    id: bpmRatioText

                    property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

                    anchors.top: bpmText.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: contentHeight
                    text: (ratio > 0) ? "+" + ratio.toFixed(2) : ratio.toFixed(2)
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Open Sans"
                    font.bold: true
                    font.pixelSize: 10
                    color: "#404040"
                }

                MixxxControls.Slider {
                    id: rateSlider

                    anchors.top: bpmRatioText.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    group: root.group
                    key: "rate"
                    bar: true
                    barColor: "#257b82"
                    barMargin: 10
                    barStart: 0.5

                    handle: Image {
                        source: "../LateNight/palemoon/sliders/knob_volume_deck.svg"
                        width: 42
                        height: 19
                    }

                    background: Image {
                        anchors.fill: parent
                        source: "../LateNight/palemoon/sliders/slider_volume_deck.svg"
                    }

                }

            }

        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.fillWidth: true
            height: 26
            color: "transparent"

            RowLayout {
                anchors.fill: parent

                Rectangle {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    implicitWidth: 68
                    implicitHeight: 26
                    color: "#121213"

                    MixxxControls.ToggleButton {
                        id: playButton

                        anchors.fill: parent
                        group: root.group
                        key: "play"
                        icon.width: 50
                        icon.height: 24

                        State {
                            name: "0"

                            PropertyChanges {
                                target: playButton
                                icon.source: "../LateNight/palemoon/buttons/btn__play_deck.svg"
                                icon.color: "#777777"
                                background.color: "transparent"
                                background.border.width: 2
                            }

                            PropertyChanges {
                                target: playButtonBgImage
                                source: "../LateNight/palemoon/buttons/btn_embedded_play.svg"
                            }

                        }

                        State {
                            name: "1"

                            PropertyChanges {
                                target: playButton
                                icon.source: "../LateNight/palemoon/buttons/btn__play_deck_active.svg"
                                icon.color: "transparent"
                                background.color: "#b24c12"
                                background.border.width: 2
                            }

                            PropertyChanges {
                                target: playButtonBgImage
                                source: "../LateNight/palemoon/buttons/btn_embedded_play_active.svg"
                            }

                        }

                        background: Rectangle {
                            anchors.fill: parent

                            data: Image {
                                id: playButtonBgImage

                                anchors.fill: parent
                            }

                        }

                    }

                }

                Item {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Item {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    implicitWidth: 50
                    implicitHeight: 20

                    Mixxx.ControlProxy {
                        id: passthroughControl

                        group: root.group
                        key: "passthrough"
                    }

                    Switch {
                        id: passthroughSwitch

                        width: parent.implicitWidth
                        height: 18
                        anchors.margins: 5
                        text: "Passthrough"
                        checked: passthroughControl.value
                        onCheckedChanged: passthroughControl.value = checked

                        indicator: Rectangle {
                            anchors.fill: parent
                            x: passthroughSwitch.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: height / 2
                            color: passthroughSwitch.checked ? "#202020" : "#121213"
                            border.color: "#404040"

                            Rectangle {
                                x: passthroughSwitch.checked ? parent.width - width : 0
                                height: passthroughSwitch.height
                                width: height
                                radius: height / 2
                                color: passthroughSwitch.checked ? "#777" : "#404040"
                                border.color: "#404040"
                            }

                        }

                    }

                }

            }

        }

    }

}
