import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Rectangle {
    id: root

    property alias show4decks: show4DecksButton.checked
    property alias maximizeLibrary: maximizeLibraryButton.checked

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

                Skin.Button {
                    id: maximizeLibraryButton

                    text: "Library"
                    activeColor: Theme.white
                    checkable: true
                }

            }

        }

        WaveformRow {
            id: deck3waveform

            group: "[Channel3]"
            width: root.width
            height: 60
            visible: root.show4decks && !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck3waveform
            }

        }

        WaveformRow {
            id: deck1waveform

            group: "[Channel1]"
            width: root.width
            height: 60
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck1waveform
            }

        }

        WaveformRow {
            id: deck2waveform

            group: "[Channel2]"
            width: root.width
            height: 60
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck2waveform
            }

        }

        WaveformRow {
            id: deck4waveform

            group: "[Channel4]"
            width: root.width
            height: 60
            visible: root.show4decks && !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck4waveform
            }

        }

        Skin.DeckRow {
            id: decks12

            leftDeckGroup: "[Channel1]"
            rightDeckGroup: "[Channel2]"
            width: parent.width - 10
            x: 5
            minimized: root.maximizeLibrary
        }

        Skin.CrossfaderRow {
            id: crossfader

            crossfaderWidth: decks12.mixer.width
            width: parent.width - 10
            x: 5
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: crossfader
            }

        }

        Skin.DeckRow {
            id: decks34

            leftDeckGroup: "[Channel3]"
            rightDeckGroup: "[Channel4]"
            width: parent.width - 10
            x: 5
            minimized: root.maximizeLibrary
            visible: root.show4decks

            FadeBehavior on visible {
                fadeTarget: decks34
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
