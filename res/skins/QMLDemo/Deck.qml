import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.15
import QtQuick.Controls 2.15
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

                MixxxControls.Slider {
                    id: volumeSlider

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    width: parent.width
                    height: parent.height
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
                        width: volumeSlider.width
                        height: volumeSlider.height
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
