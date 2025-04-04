import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import "Theme"

ApplicationWindow {
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
        id: content
        anchors.fill: parent

        Rectangle {
            id: toolbar

            width: parent.width
            height: 36
            color: Theme.toolbarBackgroundColor
            radius: 1

            RowLayout {
                anchors.fill: parent

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

                Item {
                    Layout.fillWidth: true
                }

                Skin.Button {
                    id: showDevToolsButton

                    text: "Develop"
                    activeColor: Theme.white
                    checkable: true
                    checked: devToolsWindow.visible
                    onClicked: {
                        if (devToolsWindow.visible)
                            devToolsWindow.close();
                        else
                            devToolsWindow.show();
                    }

                    DeveloperToolsWindow {
                        id: devToolsWindow

                        width: 640
                        height: 480
                    }
                }

                Skin.Button {
                    id: showPreferencesButton

                    implicitWidth: implicitHeight

                    icon.source: "images/gear.svg"
                    icon.width: 16
                    icon.height: 16

                    activeColor: Theme.white
                    checked: settingsPopup.opened
                    onPressAndHold: {
                        Mixxx.PreferencesDialog.show();
                    }
                    onClicked: {
                        if (!settingsPopup.opened) {
                            settingsPopup.open()
                        }
                    }
                }
            }
        }

        Skin.WaveformDisplay {
            id: deck3waveform

            group: "[Channel3]"
            width: root.width
            height: 120
            visible: root.show4decks && !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck3waveform
            }
        }

        Skin.WaveformDisplay {
            id: deck1waveform

            group: "[Channel1]"
            width: root.width
            height: 120
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck1waveform
            }
        }

        Skin.WaveformDisplay {
            id: deck2waveform

            group: "[Channel2]"
            width: root.width
            height: 120
            visible: !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck2waveform
            }
        }

        Skin.WaveformDisplay {
            id: deck4waveform

            group: "[Channel4]"
            width: root.width
            height: 120
            visible: root.show4decks && !root.maximizeLibrary

            FadeBehavior on visible {
                fadeTarget: deck4waveform
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

            Skin.FadeBehavior on visible {
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

            Skin.FadeBehavior on visible {
                fadeTarget: decks34
            }
        }

        Skin.SamplerRow {
            id: samplers

            width: parent.width
            visible: root.showSamplers

            Skin.FadeBehavior on visible {
                fadeTarget: samplers
            }
        }

        Skin.EffectRow {
            id: effects

            width: parent.width
            visible: root.showEffects

            Skin.FadeBehavior on visible {
                fadeTarget: effects
            }
        }

        Skin.Library {
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

    Skin.Settings {
        id: settingsPopup
        // anchors.centerIn: Overlay.overlay
        // focus: true
        // closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        // parent: Overlay.overlay
        width: Math.max(1400, parent.width*0.8)
        height: Math.max(680, parent.height*0.7)

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        modal: true
        Overlay.modal: Rectangle {
            color: '#000000'
            anchors.fill: parent
            property real radius: 12
            GaussianBlur {
                anchors.fill: parent
                source: content
                radius: Math.max(0, parent.radius)
                samples: 16
                deviation: 4
            }
        }
    }
    // Popup {
    //     id: settingsPopup
    // }

    Component.onCompleted: {
        settingsPopup.open()
    }
}
