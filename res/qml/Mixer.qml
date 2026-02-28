import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts
import "Theme"

Item {
    id: root

    required property var groups
    property bool show4decks: false

    implicitHeight: content.height + crossfader.height
    implicitWidth: 190

    Skin.SectionBackground {
        anchors.fill: parent
    }
    Column {
        anchors.fill: parent

        Item {
            id: content

            height: (root.show4decks ? eqDeck1.height * 2 : eqDeck1.height) + 10
            width: root.implicitWidth

            Skin.SectionBackground {
                anchors.fill: parent
            }
            Item {
                anchors.fill: parent
                anchors.margins: 5

                states: [
                    State {
                        name: "2decks"
                        when: root.show4decks

                        AnchorChanges {
                            target: eqDeck1

                            anchors {
                                left: eqDeck3.right
                                top: parent.top
                            }
                        }
                        AnchorChanges {
                            target: mixerDeck1

                            anchors {
                                left: mixerDeck3.right
                                top: eqDeck1.bottom
                            }
                        }
                        AnchorChanges {
                            target: mixerDeck2

                            anchors {
                                right: mixerDeck4.left
                                top: eqDeck2.bottom
                            }
                        }
                        AnchorChanges {
                            target: eqDeck2

                            anchors {
                                right: eqDeck4.left
                                top: parent.top
                            }
                        }
                    }
                ]

                Skin.EqColumn {
                    id: eqDeck3

                    group: root.groups[2]
                    visible: root.show4decks
                    width: 42

                    anchors {
                        left: parent.left
                        rightMargin: 4
                        top: parent.top
                    }
                }
                Skin.EqColumn {
                    id: eqDeck1

                    group: root.groups[0]
                    width: 42

                    anchors {
                        left: parent.left
                        leftMargin: root.show4decks ? 4 : 0
                        rightMargin: 4
                        top: parent.top
                    }
                }
                Skin.MixerColumn {
                    id: mixerDeck3

                    group: root.groups[2]
                    height: eqDeck2.height
                    visible: root.show4decks
                    width: 42

                    anchors {
                        left: parent.left
                        rightMargin: 4
                        top: eqDeck3.bottom
                    }
                }
                Skin.MixerColumn {
                    id: mixerDeck1

                    group: root.groups[0]
                    height: eqDeck1.height
                    width: 42

                    anchors {
                        left: eqDeck1.right
                        leftMargin: 4
                        rightMargin: 2
                        top: parent.top
                    }
                }
                Skin.MixerColumn {
                    id: mixerDeck2

                    group: root.groups[1]
                    height: eqDeck2.height
                    width: 42

                    anchors {
                        leftMargin: 2
                        right: eqDeck2.left
                        rightMargin: 4
                        top: parent.top
                    }
                }
                Skin.MixerColumn {
                    id: mixerDeck4

                    group: root.groups[3]
                    height: eqDeck2.height
                    visible: root.show4decks
                    width: 42

                    anchors {
                        leftMargin: 4
                        right: parent.right
                        top: eqDeck4.bottom
                    }
                }
                Skin.EqColumn {
                    id: eqDeck2

                    group: root.groups[1]
                    width: 42

                    anchors {
                        right: parent.right
                        // leftMargin: 4
                        rightMargin: root.show4decks ? 4 : 0
                        top: parent.top
                    }
                }
                Skin.EqColumn {
                    id: eqDeck4

                    group: root.groups[3]
                    visible: root.show4decks
                    width: 42

                    anchors {
                        leftMargin: 4
                        right: parent.right
                        top: parent.top
                    }
                }
            }
        }
        Item {
            id: crossfader

            height: 40
            width: root.implicitWidth

            Skin.SectionBackground {
                anchors.fill: parent
            }
            Item {
                anchors.fill: parent
                anchors.margins: 3

                Skin.SectionBackground {
                    anchors.fill: parent
                }
                GridLayout {
                    id: leftDeckAssignment

                    anchors.left: parent.left
                    columnSpacing: 0
                    rowSpacing: 0
                    width: root.show4decks ? 34 : 17

                    Repeater {
                        model: root.show4decks ? 4 : 2

                        Item {
                            required property int index

                            Layout.column: show4decks ? index % 2 : 0
                            Layout.row: root.show4decks ? parseInt(index / 2) : index
                            implicitHeight: 17
                            implicitWidth: 17

                            Skin.SectionBackground {
                                anchors.fill: parent
                            }
                            Skin.ControlButton {
                                id: deckButton

                                activeColor: Theme.deckActiveColor
                                group: `[Channel${index + 1}]`
                                key: "orientation_left"
                                text: `${index + 1}`
                                toggleable: true

                                anchors {
                                    fill: parent
                                    margins: 1
                                }
                            }
                        }
                    }
                }
                Skin.ControlFader {
                    id: crossfaderSlider

                    barColor: Theme.crossfaderBarColor
                    barStart: 0.5
                    bg: Theme.imgCrossfaderBackground
                    fg: Theme.imgCrossfaderHandle
                    group: "[Master]"
                    key: "crossfader"
                    orientation: Qt.Horizontal

                    anchors {
                        left: leftDeckAssignment.right
                        right: rightDeckAssignment.left
                        verticalCenter: parent.verticalCenter
                    }
                    handleImage {
                        height: 34
                    }
                }
                GridLayout {
                    id: rightDeckAssignment

                    anchors.right: parent.right
                    columnSpacing: 0
                    rowSpacing: 0
                    width: root.show4decks ? 34 : 17

                    Repeater {
                        model: root.show4decks ? 4 : 2

                        Item {
                            required property int index

                            Layout.column: show4decks ? index % 2 : 0
                            Layout.row: root.show4decks ? parseInt(index / 2) : index
                            implicitHeight: 17
                            implicitWidth: 17

                            Skin.SectionBackground {
                                anchors.fill: parent
                            }
                            Skin.ControlButton {
                                id: deckButton

                                activeColor: Theme.deckActiveColor
                                group: `[Channel${index + 1}]`
                                key: "orientation_right"
                                text: `${index + 1}`
                                toggleable: true

                                anchors {
                                    fill: parent
                                    margins: 1
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
