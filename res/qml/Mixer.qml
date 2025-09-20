import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts
import "Theme"

Item {
    id: root

    required property var groups
    property bool show4decks: false

    implicitWidth: 190
    implicitHeight: content.height + crossfader.height

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Column {
        anchors.fill: parent

        Item {
            id: content

            width: root.implicitWidth
            height: (root.show4decks ? eqDeck1.height * 2 : eqDeck1.height) + 10

            Skin.SectionBackground {
                anchors.fill: parent
            }
            Item {
                anchors.fill: parent
                anchors.margins: 5

                Skin.EqColumn {
                    id: eqDeck3
                    visible: root.show4decks
                    width: 42
                    group: root.groups[2]
                    anchors {
                        left: parent.left
                        top: parent.top
                        rightMargin: 4
                    }
                }

                Skin.EqColumn {
                    id: eqDeck1
                    width: 42
                    group: root.groups[0]
                    anchors {
                        left: parent.left
                        top: parent.top
                        leftMargin: root.show4decks ? 4 : 0
                        rightMargin: 4
                    }
                }

                Skin.MixerColumn {
                    id: mixerDeck3
                    visible: root.show4decks
                    width: 42
                    height: eqDeck2.height
                    group: root.groups[2]
                    anchors {
                        left: parent.left
                        top: eqDeck3.bottom
                        rightMargin: 4
                    }
                }

                Skin.MixerColumn {
                    id: mixerDeck1
                    width: 42
                    height: eqDeck1.height
                    group: root.groups[0]
                    anchors {
                        left: eqDeck1.right
                        top: parent.top
                        leftMargin: 4
                        rightMargin: 2
                    }
                }

                Skin.MixerColumn {
                    id: mixerDeck2
                    width: 42
                    height: eqDeck2.height
                    group: root.groups[1]
                    anchors {
                        right: eqDeck2.left
                        top: parent.top
                        leftMargin: 2
                        rightMargin: 4
                    }
                }

                Skin.MixerColumn {
                    id: mixerDeck4
                    visible: root.show4decks
                    width: 42
                    height: eqDeck2.height
                    group: root.groups[3]
                    anchors {
                        right: parent.right
                        top: eqDeck4.bottom
                        leftMargin: 4
                    }
                }

                Skin.EqColumn {
                    id: eqDeck2
                    width: 42
                    group: root.groups[1]
                    anchors {
                        right: parent.right
                        top: parent.top
                        // leftMargin: 4
                        rightMargin: root.show4decks ? 4 : 0
                    }
                }

                Skin.EqColumn {
                    id: eqDeck4
                    visible: root.show4decks
                    width: 42
                    group: root.groups[3]
                    anchors {
                        right: parent.right
                        top: parent.top
                        leftMargin: 4
                    }
                }

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
            }
        }
        Item {
            id: crossfader

            width: root.implicitWidth
            height: 40

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

                    columnSpacing: 0
                    rowSpacing: 0

                    width: root.show4decks ? 34 : 17
                    anchors.left: parent.left

                    Repeater {
                        model: root.show4decks ? 4 : 2
                        Item {
                            required property int index
                            Layout.column: show4decks ? index % 2 : 0
                            Layout.row: root.show4decks ? parseInt(index / 2) : index

                            implicitWidth: 17
                            implicitHeight: 17

                            Skin.SectionBackground {
                                anchors.fill: parent
                            }

                            Skin.ControlButton {
                                id: deckButton
                                anchors {
                                    fill: parent
                                    margins: 1
                                }

                                group: `[Channel${index+1}]`
                                key: "orientation_left"
                                text: `${index+1}`
                                toggleable: true
                                activeColor: Theme.deckActiveColor
                            }
                        }
                    }
                }
                Skin.ControlFader {
                    id: crossfaderSlider

                    anchors {
                        left: leftDeckAssignment.right
                        right: rightDeckAssignment.left
                        verticalCenter: parent.verticalCenter
                    }

                    orientation: Qt.Horizontal
                    group: "[Master]"
                    key: "crossfader"
                    barColor: Theme.crossfaderBarColor
                    barStart: 0.5
                    fg: Theme.imgCrossfaderHandle
                    bg: Theme.imgCrossfaderBackground
                    handleImage {
                        height: 34
                    }
                }
                GridLayout {
                    id: rightDeckAssignment

                    columnSpacing: 0
                    rowSpacing: 0

                    width: root.show4decks ? 34 : 17
                    anchors.right: parent.right

                    Repeater {
                        model: root.show4decks ? 4 : 2
                        Item {
                            required property int index
                            Layout.column: show4decks ? index % 2 : 0
                            Layout.row: root.show4decks ? parseInt(index / 2) : index

                            implicitWidth: 17
                            implicitHeight: 17

                            Skin.SectionBackground {
                                anchors.fill: parent
                            }

                            Skin.ControlButton {
                                id: deckButton
                                anchors {
                                    fill: parent
                                    margins: 1
                                }

                                group: `[Channel${index+1}]`
                                key: "orientation_right"
                                text: `${index+1}`
                                toggleable: true
                                activeColor: Theme.deckActiveColor
                            }
                        }
                    }
                }
            }
        }
    }
}
