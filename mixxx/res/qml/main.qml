import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import "Theme"

ApplicationWindow {
    id: root

    property alias maximizeLibrary: maximizeLibraryButton.checked
    property alias show4decks: show4DecksButton.checked
    property alias showEffects: showEffectsButton.checked
    property alias showSamplers: showSamplersButton.checked

    color: Theme.backgroundColor
    height: 1080
    visible: true
    width: 1920

    Column {
        id: content
        anchors.fill: parent

        move: Transition {
            NumberAnimation {
                duration: 150
                properties: "x,y"
            }
        }

        Rectangle {
            id: toolbar
            color: Theme.toolbarBackgroundColor
            height: 36
            radius: 1
            width: parent.width

            RowLayout {
                anchors.fill: parent

                Skin.Button {
                    id: show4DecksButton
                    activeColor: Theme.white
                    checkable: true
                    text: "4 Decks"
                }
                Skin.Button {
                    id: maximizeLibraryButton
                    activeColor: Theme.white
                    checkable: true
                    text: "Library"
                }
                Skin.Button {
                    id: showEffectsButton
                    activeColor: Theme.white
                    checkable: true
                    text: "Effects"
                }
                Skin.Button {
                    id: showSamplersButton
                    activeColor: Theme.white
                    checkable: true
                    text: "Sampler"
                }
                Item {
                    Layout.fillWidth: true
                }
                Skin.Button {
                    id: showDevToolsButton
                    activeColor: Theme.white
                    checkable: true
                    checked: devToolsWindow.visible
                    text: "Develop"

                    onClicked: {
                        if (devToolsWindow.visible)
                            devToolsWindow.close();
                        else
                            devToolsWindow.show();
                    }

                    DeveloperToolsWindow {
                        id: devToolsWindow
                        height: 480
                        width: 640
                    }
                }
                Skin.Button {
                    id: showPreferencesButton
                    activeColor: Theme.white
                    checked: settingsPopup.opened
                    icon.height: 16
                    icon.source: "images/gear.svg"
                    icon.width: 16
                    implicitWidth: implicitHeight

                    onClicked: {
                        if (!settingsPopup.opened) {
                            settingsPopup.open();
                        }
                    }
                    onPressAndHold: {
                        Mixxx.PreferencesDialog.show();
                    }
                }
            }
        }
        Skin.WaveformDisplay {
            id: deck3waveform
            group: "[Channel3]"
            height: 120
            visible: root.show4decks && !root.maximizeLibrary
            width: root.width

            FadeBehavior on visible  {
                fadeTarget: deck3waveform
            }
        }
        Skin.WaveformDisplay {
            id: deck1waveform
            group: "[Channel1]"
            height: 120
            visible: !root.maximizeLibrary
            width: root.width

            FadeBehavior on visible  {
                fadeTarget: deck1waveform
            }
        }
        Skin.WaveformDisplay {
            id: deck2waveform
            group: "[Channel2]"
            height: 120
            visible: !root.maximizeLibrary
            width: root.width

            FadeBehavior on visible  {
                fadeTarget: deck2waveform
            }
        }
        Skin.WaveformDisplay {
            id: deck4waveform
            group: "[Channel4]"
            height: 120
            visible: root.show4decks && !root.maximizeLibrary
            width: root.width

            FadeBehavior on visible  {
                fadeTarget: deck4waveform
            }
        }
        Skin.DeckRow {
            id: decks12
            leftDeckGroup: "[Channel1]"
            minimized: root.maximizeLibrary
            rightDeckGroup: "[Channel2]"
            width: parent.width
        }
        Skin.CrossfaderRow {
            id: crossfader
            crossfaderWidth: decks12.mixer.width
            visible: !root.maximizeLibrary
            width: parent.width

            Skin.FadeBehavior on visible  {
                fadeTarget: crossfader
            }
        }
        Skin.DeckRow {
            id: decks34
            leftDeckGroup: "[Channel3]"
            minimized: root.maximizeLibrary
            rightDeckGroup: "[Channel4]"
            visible: root.show4decks
            width: parent.width

            Skin.FadeBehavior on visible  {
                fadeTarget: decks34
            }
        }
        Skin.SamplerRow {
            id: samplers
            visible: root.showSamplers
            width: parent.width

            Skin.FadeBehavior on visible  {
                fadeTarget: samplers
            }
        }
        Skin.EffectRow {
            id: effects
            visible: root.showEffects
            width: parent.width

            Skin.FadeBehavior on visible  {
                fadeTarget: effects
            }
        }
        Skin.Library {
            height: parent.height - y
            width: parent.width
        }
    }
    Skin.Settings {
        id: settingsPopup
        height: Math.max(840, parent.height * 0.7)
        modal: true
        width: Math.max(1400, parent.width * 0.8)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        Overlay.modal: Rectangle {
            id: overlayModal
            property real radius: 12

            readonly property bool hasHardwareAcceleration: Mixxx.Config.useAcceleration()

            anchors.fill: parent
            color: Qt.alpha('#00000010', hasHardwareAcceleration ? 1.0 : 0.6)

            Repeater {
                model: hasHardwareAcceleration ? 1 : 0
                GaussianBlur {
                    anchors.fill: overlayModal
                    deviation: 4
                    radius: Math.max(0, overlayModal.radius)
                    samples: 16
                    source: content
                }
            }
        }
    }
}
