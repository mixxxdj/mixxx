import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11

Rectangle {
    id: root

    width: 1920
    height: 1080
    color: "#1e1e20"
    visible: true

    Column {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            id: toolbar

            width: parent.width
            height: 36
            color: "#151517"
            border.color: "#020202"
            border.width: 1
            radius: 1

            Row {
                padding: 5
                spacing: 5

                Button {
                    id: decksCDToggle

                    width: 100
                    height: 26
                    text: "4 DECKS"
                    font.family: "Open Sans"
                    font.bold: true
                    font.pixelSize: 11
                    onClicked: decksCD.stateHidden = !decksCD.stateHidden
                    states: [
                        State {
                            when: decksCD.stateHidden

                            PropertyChanges {
                                target: decksCDToggle
                                background.color: "#151517"
                                contentItem.color: "#777777"
                            }

                            PropertyChanges {
                                target: decksCDToggleBgImage
                                source: "../LateNight/palemoon/buttons/btn___active.svg"
                            }

                        },
                        State {
                            when: !decksCD.stateHidden

                            PropertyChanges {
                                target: decksCDToggle
                                background.color: "#777777"
                                contentItem.color: "#000000"
                            }

                            PropertyChanges {
                                target: decksCDToggleBgImage
                                source: "../LateNight/palemoon/buttons/btn__.svg"
                            }

                        }
                    ]

                    contentItem: Text {
                        text: decksCDToggle.text
                        font: decksCDToggle.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        anchors.fill: parent
                        border.width: 2

                        Image {
                            id: decksCDToggleBgImage

                            anchors.fill: parent
                        }

                    }

                }

            }

        }

        DeckRow {
            id: decksAB

            width: root.width
            height: 180
            leftDeck: "[Channel1]"
            rightDeck: "[Channel2]"
        }

        DeckRow {
            id: decksCD

            property bool stateHidden: true

            function toggle() {
                stateHidden = !stateHidden;
            }

            width: root.width
            height: 180
            leftDeck: "[Channel3]"
            rightDeck: "[Channel4]"
            states: [
                State {
                    when: !decksCD.stateHidden

                    PropertyChanges {
                        target: decksCD
                        opacity: 1
                        visible: true
                    }

                },
                State {
                    when: decksCD.stateHidden

                    PropertyChanges {
                        target: decksCD
                        opacity: 0
                        visible: false
                    }

                }
            ]
            transitions: [
                Transition {
                    enabled: !decksCD.stateHidden

                    SequentialAnimation {
                        NumberAnimation {
                            target: decksCD
                            property: "opacity"
                            duration: 150
                        }

                        PropertyAction {
                            target: decksCD
                            property: "visible"
                        }

                    }

                },
                Transition {
                    enabled: decksCD.stateHidden

                    SequentialAnimation {
                        PropertyAction {
                            target: decksCD
                            property: "visible"
                        }

                        NumberAnimation {
                            target: decksCD
                            property: "opacity"
                            duration: 150
                        }

                    }

                }
            ]
        }

        Library {
            width: parent.width
            height: parent.height - y
        }

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 150
            }

        }

    }

}
