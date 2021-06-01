import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Rectangle {
    id: root

    property alias show4decks: show4DecksButton.checked

    width: 1920
    height: 1080
    color: Theme.backgroundColor
    visible: true

    Column {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            id: toolbar

            width: parent.width
            height: 36
            color: Theme.toolbarBackgroundColor
            radius: 1

            Row {
                padding: 5
                spacing: 5

                Skin.Button {
                    id: show4DecksButton

                    text: "4 Decks"
                    activeColor: Theme.white
                    checkable: true
                }

            }

        }

        Skin.DeckRow {
            id: decks12

            leftDeckGroup: "[Channel1]"
            rightDeckGroup: "[Channel2]"
            width: parent.width - 10
            x: 5
        }

        Skin.CrossfaderRow {
            id: crossfader

            crossfaderWidth: decks12.mixer.width
            width: parent.width - 10
            x: 5
        }

        Skin.DeckRow {
            id: decks34

            leftDeckGroup: "[Channel3]"
            rightDeckGroup: "[Channel4]"
            width: parent.width - 10
            x: 5

            states: Skin.HiddenState {
                when: !root.show4decks
                target: decks34
            }

            transitions: Skin.HiddenTransition {
                target: decks34
            }

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
