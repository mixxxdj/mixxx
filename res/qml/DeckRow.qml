import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    property string leftDeckGroup // required
    property string rightDeckGroup // required
    property alias mixer: mixer
    property bool minimized: false

    implicitHeight: mixer.height
    states: [
        State {
            when: root.minimized
            name: "minimized"

            PropertyChanges {
                target: mixer
                visible: false
            }

            PropertyChanges {
                target: root
                implicitHeight: 66
            }

            AnchorChanges {
                target: leftDeck
                anchors.right: mixer.horizontalCenter
            }

            AnchorChanges {
                target: rightDeck
                anchors.left: mixer.horizontalCenter
            }

        },
        State {
            // This State can't be deduplicated by making the first one
            // reversible, because for decks 3/4 the mixer may already be
            // hidden (since the whole deck row is already hidden). In that
            // case, disabling the minimized state would not show the mixer
            // again.
            when: !root.minimized
            name: "maximized"

            PropertyChanges {
                target: mixer
                visible: true
            }

            PropertyChanges {
                target: root
                implicitHeight: mixer.height
            }

            AnchorChanges {
                target: leftDeck
                anchors.right: mixer.left
            }

            AnchorChanges {
                target: rightDeck
                anchors.left: mixer.right
            }

        }
    ]

    Deck {
        id: leftDeck

        minimized: root.minimized
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        group: root.leftDeckGroup
    }

    Mixer {
        id: mixer

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        leftDeckGroup: root.leftDeckGroup
        rightDeckGroup: root.rightDeckGroup

        FadeBehavior on visible {
            fadeTarget: mixer
        }

    }

    Deck {
        id: rightDeck

        minimized: root.minimized
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        group: root.rightDeckGroup
    }

    transitions: Transition {
        to: "minimized"
        reversible: true

        SequentialAnimation {
            AnchorAnimation {
                targets: [leftDeck, rightDeck]
                duration: 150
            }

            PropertyAnimation {
                target: root
                property: "implicitHeight"
                duration: 150
            }

        }

    }

}
