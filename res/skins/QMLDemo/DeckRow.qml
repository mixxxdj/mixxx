import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Item {
    id: root

    required property string leftDeck
    required property string rightDeck

    RowLayout {
        anchors.fill: parent
        Layout.margins: 0

        Rectangle {
            color: "#121213"
            implicitWidth: 68
            implicitHeight: 26
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            MixxxControls.ToggleButton {
                id: playButtonDeck1

                anchors.fill: parent
                group: root.leftDeck
                key: "play"
                icon.width: 50
                icon.height: 24

                State {
                    name: "0"

                    PropertyChanges {
                        target: playButtonDeck1
                        icon.source: "../LateNight/palemoon/buttons/btn__play_deck.svg"
                        icon.color: "#777777"
                        background.color: "transparent"
                        background.border.width: 2
                    }

                    PropertyChanges {
                        target: playButtonDeck1BgImage
                        source: "../LateNight/palemoon/buttons/btn_embedded_play.svg"
                    }

                }

                State {
                    name: "1"

                    PropertyChanges {
                        target: playButtonDeck1
                        icon.source: "../LateNight/palemoon/buttons/btn__play_deck_active.svg"
                        icon.color: "transparent"
                        background.color: "#b24c12"
                        background.border.width: 2
                    }

                    PropertyChanges {
                        target: playButtonDeck1BgImage
                        source: "../LateNight/palemoon/buttons/btn_embedded_play_active.svg"
                    }

                }

                background: Rectangle {
                    anchors.fill: parent

                    data: Image {
                        id: playButtonDeck1BgImage

                        anchors.fill: parent
                    }

                }

            }

        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            implicitWidth: 300
            implicitHeight: 180
            color: "#1e1e20"
            border.color: "#121213"
            border.width: 2
            radius: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                EqColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.leftDeck
                }

                MixerColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.leftDeck
                }

                MixerColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.rightDeck
                }

                EqColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.rightDeck
                }

            }

        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            implicitWidth: 68
            implicitHeight: 26
            color: "#121213"

            MixxxControls.ToggleButton {
                id: playButtonDeck2

                anchors.fill: parent
                group: root.rightDeck
                key: "play"
                icon.width: 50
                icon.height: 24

                State {
                    name: "0"

                    PropertyChanges {
                        target: playButtonDeck2
                        icon.source: "../LateNight/palemoon/buttons/btn__play_deck.svg"
                        icon.color: "#777777"
                        background.color: "transparent"
                        background.border.width: 2
                    }

                    PropertyChanges {
                        target: playButtonDeck2BgImage
                        source: "../LateNight/palemoon/buttons/btn_embedded_play.svg"
                    }

                }

                State {
                    name: "1"

                    PropertyChanges {
                        target: playButtonDeck2
                        icon.source: "../LateNight/palemoon/buttons/btn__play_deck_active.svg"
                        icon.color: "transparent"
                        background.color: "#b24c12"
                        background.border.width: 2
                    }

                    PropertyChanges {
                        target: playButtonDeck2BgImage
                        source: "../LateNight/palemoon/buttons/btn_embedded_play_active.svg"
                    }

                }

                background: Rectangle {
                    anchors.fill: parent

                    data: Image {
                        id: playButtonDeck2BgImage

                        anchors.fill: parent
                    }

                }

            }

        }

    }

}
