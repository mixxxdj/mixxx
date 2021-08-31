import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Rectangle {
    id: root

    property alias show4decks: show4DecksButton.checked
    property alias showEffects: showEffectsButton.checked
    property alias showSamplers: showSamplersButton.checked
    property alias maximizeLibrary: maximizeLibraryButton.checked

    width: 1920
    height: 1080
    color: Theme.backgroundColor
    visible: true

    Column {
        anchors.fill: parent

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

                Skin.Button {
                    id: showEffectsButton

                    text: "Effects"
                    activeColor: Theme.white
                    checkable: true
                }

                Skin.Button {
                    id: showSamplersButton

                    text: "Sampler"
                    activeColor: Theme.white
                    checkable: true
                }

            }

        }

        Skin.DeckRow {
            id: decks12

            leftDeckGroup: "[Channel1]"
            rightDeckGroup: "[Channel2]"
            width: parent.width
            minimized: root.maximizeLibrary
        }

        Skin.CrossfaderRow {
            id: crossfader

            crossfaderWidth: decks12.mixer.width
            width: parent.width
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: crossfader
            }

        }

        Skin.DeckRow {
            id: decks34

            leftDeckGroup: "[Channel3]"
            rightDeckGroup: "[Channel4]"
            width: parent.width
            minimized: root.maximizeLibrary
            visible: root.show4decks

            FadeBehavior on visible {
                fadeTarget: decks34
            }

        }

        SamplerRow {
            id: samplers

            width: parent.width
            visible: root.showSamplers

            FadeBehavior on visible {
                fadeTarget: samplers
            }

        }

        EffectRow {
            id: effects

            width: parent.width
            visible: root.showEffects

            FadeBehavior on visible {
                fadeTarget: effects
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
