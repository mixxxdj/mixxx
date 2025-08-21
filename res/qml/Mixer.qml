import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts
import "Theme"

Item {
    id: root

    required property var groups
    property bool show4decks: false

    implicitWidth: content.width + 10
    implicitHeight: content.height + crossfader.height + 20

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Item {
        id: content

        width: 46 * 4
        height: root.show4decks ? eqDeck1.height * 2 : eqDeck1.height

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 5

        Skin.EqColumn {
            id: eqDeck3
            visible: root.show4decks
            width: 46
            group: root.groups[2]
            anchors {
                left: parent.left
                top: parent.top
            }
        }

        Skin.EqColumn {
            id: eqDeck1
            width: 46
            group: root.groups[0]
            anchors {
                left: parent.left
                top: parent.top
            }
        }

        Skin.MixerColumn {
            id: mixerDeck3
            visible: root.show4decks
            width: 46
            height: eqDeck2.height
            group: root.groups[2]
            anchors {
                left: parent.left
                top: eqDeck3.bottom
            }
        }

        Skin.MixerColumn {
            id: mixerDeck1
            width: 46
            height: eqDeck1.height
            group: root.groups[0]
            anchors {
                left: eqDeck1.right
                top: parent.top
            }
        }

        Skin.MixerColumn {
            id: mixerDeck2
            width: 46
            height: eqDeck2.height
            group: root.groups[1]
            anchors {
                right: eqDeck2.left
                top: parent.top
            }
        }

        Skin.MixerColumn {
            id: mixerDeck4
            visible: root.show4decks
            width: 46
            height: eqDeck2.height
            group: root.groups[3]
            anchors {
                right: parent.right
                top: eqDeck4.bottom
            }
        }

        Skin.EqColumn {
            id: eqDeck2
            width: 46
            group: root.groups[1]
            anchors {
                right: parent.right
                top: parent.top
            }
        }

        Skin.EqColumn {
            id: eqDeck4
            visible: root.show4decks
            width: 46
            group: root.groups[3]
            anchors {
                right: parent.right
                top: parent.top
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
    RowLayout {
        id: crossfader

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 5
        height: crossfaderSlider.implicitHeight
        GridLayout {
            id: leftDeckAssignment
            columnSpacing: 1
            rowSpacing: 1
            Repeater {
                model: root.show4decks ? 4 : 2
                Skin.ControlButton {
                    required property int index
                    id: deckButton

                    Layout.column: show4decks ? index % 2 : 0
                    Layout.row: root.show4decks ? parseInt(index / 2) : index

                    implicitWidth: 17
                    implicitHeight: 17

                    group: `[Channel${index+1}]`
                    key: "orientation_left"
                    text: `${index+1}`
                    toggleable: true
                    activeColor: Theme.deckActiveColor
                }
            }
        }
        Skin.ControlFader {
            id: crossfaderSlider
            Layout.fillWidth: true
            Layout.leftMargin: 2
            Layout.rightMargin: 2
            // height: leftDeckAssignment.height

            orientation: Qt.Horizontal
            group: "[Master]"
            key: "crossfader"
            barColor: Theme.crossfaderBarColor
            barStart: 0.5
            fg: Theme.imgCrossfaderHandle
            bg: Theme.imgCrossfaderBackground
        }
        GridLayout {
            id: rightDeckAssignment
            columnSpacing: 1
            rowSpacing: 1
            Repeater {
                model: root.show4decks ? 4 : 2
                Skin.ControlButton {
                    required property int index
                    id: deckButton

                    Layout.column: show4decks ? index % 2 : 0
                    Layout.row: root.show4decks ? parseInt(index / 2) : index

                    implicitWidth: 17
                    implicitHeight: 17

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
