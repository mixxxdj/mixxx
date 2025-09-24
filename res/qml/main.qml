import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"

ApplicationWindow {
    id: root

    property alias maximizeLibrary: maximizeLibraryButton.checked
    property alias show4decks: show4DecksButton.checked
    property alias showEffects: showEffectsButton.checked
    property alias showSamplers: showSamplersButton.checked
    property alias editDeck: editDeckButton.checked

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
                    id: showAuxButton
                    activeColor: Theme.white
                    checkable: true
                    text: "Aux"
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
                    id: editDeckButton
                    activeColor: Theme.white
                    checkable: true
                    text: "Edit"
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
        SplitView {
            id: splitView
            width: parent.width
            height: parent.height - y

            orientation: Qt.Vertical

            handle: Rectangle {
                id: handleDelegate
                implicitWidth: 8
                implicitHeight: 4
                color: Theme.libraryPanelSplitterBackground
                clip: true
                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.libraryPanelSplitterHandleActive : Theme.libraryPanelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 3

                RowLayout {
                    anchors.centerIn: parent
                    Repeater {
                        model: 3
                        Rectangle {
                            width: handleSize
                            height: handleSize
                            radius: handleSize
                            color: handleColor
                        }
                    }
                }

                containmentMask: Item {
                    x: (handleDelegate.width - width) / 2
                    height: 8
                    width: splitView.width
                }
            }

            Item {
                id: waveforms
                SplitView.preferredHeight: root.maximizeLibrary ? 120 : 240
                visible: !root.maximizeLibrary
                Skin.WaveformDisplay {
                    id: deck3waveform

                    anchors.top: parent.top

                    group: "[Channel3]"
                    height: parent.height / 4
                    visible: root.show4decks
                    width: root.width

                    FadeBehavior on visible {
                        fadeTarget: deck3waveform
                    }
                }

                Skin.WaveformDisplay {
                    id: deck1waveform

                    anchors.top: root.show4decks ? deck3waveform.bottom : parent.top

                    group: "[Channel1]"
                    height: parent.height / (root.show4decks ? 4 : 2)
                    width: root.width
                }
                Skin.WaveformDisplay {
                    id: deck2waveform

                    anchors.bottom: root.show4decks ? deck4waveform.top : parent.bottom

                    group: "[Channel2]"
                    height: parent.height / (root.show4decks ? 4 : 2)
                    width: root.width
                }
                Skin.WaveformDisplay {
                    id: deck4waveform

                    anchors.bottom: parent.bottom

                    group: "[Channel4]"
                    height: parent.height / 4
                    visible: root.show4decks
                    width: root.width

                    FadeBehavior on visible {
                        fadeTarget: deck4waveform
                    }
                }
                Rectangle {
                    width: 125
                    anchors {
                        top: parent.top
                        left: parent.left
                        bottom:parent.bottom
                    }
                    gradient: Gradient {
                        orientation: Gradient.Horizontal

                        GradientStop {
                            position: 0
                            color: Theme.darkGray
                        }

                        GradientStop {
                            position: 1
                            color: 'transparent'
                        }
                    }
                }
                Rectangle {
                    width: 125
                    anchors {
                        top: parent.top
                        right: parent.right
                        bottom:parent.bottom
                    }
                    gradient: Gradient {
                        orientation: Gradient.Horizontal

                        GradientStop {
                            position: 0
                            color: 'transparent'
                        }

                        GradientStop {
                            position: 1
                            color: Theme.darkGray
                        }
                    }
                }

                FadeBehavior on visible {
                    fadeTarget: waveforms
                }
            }

            Item {
                SplitView.fillHeight: true
                Deck {
                    id: deck1

                    anchors {
                        left: parent.left
                        right: mixer.left
                    }

                    minimized: root.maximizeLibrary
                    height: root.maximizeLibrary ? 80 : root.show4decks ? mixer.height / 2 : mixer.height
                    group: "[Channel1]"

                    editMode: root.editDeck

                    states: [
                        State {
                            when: root.maximizeLibrary
                            AnchorChanges { target: deck1; anchors.right: parent.horizontalCenter}
                        }
                    ]

                    Behavior on height {
                        SpringAnimation {
                            id: deck1HeightAnimation
                            duration: 500
                            spring: 2
                            damping: 0.2
                        }
                    }
                }

                Mixer {
                    id: mixer
                    show4decks: root.show4decks
                    groups: [
                             deck1.group,
                             deck2.group,
                             deck3.group,
                             deck4.group
                    ]

                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: !root.maximizeLibrary

                    FadeBehavior on visible {
                        fadeTarget: mixer
                    }

                    Behavior on height {
                        SpringAnimation {
                            id: mixerHeightAnimation
                            duration: 500
                            spring: 2
                            damping: 0.2
                        }
                    }
                }

                Deck {
                    id: deck2

                    anchors {
                        left: mixer.right
                        right: parent.right
                    }

                    minimized: root.maximizeLibrary
                    height: root.maximizeLibrary ? 80 : root.show4decks ? mixer.height / 2 : mixer.height
                    group: "[Channel2]"

                    editMode: root.editDeck

                    states: [
                        State {
                            when: root.maximizeLibrary
                            AnchorChanges { target: deck2; anchors.left: parent.horizontalCenter}
                        }
                    ]

                    Behavior on height {
                        SpringAnimation {
                            id: deck2HeightAnimation
                            duration: 500
                            spring: 2
                            damping: 0.2
                        }
                    }
                }

                Deck {
                    id: deck3

                    anchors {
                        top: deck1.bottom
                        left: parent.left
                        right: mixer.left
                    }

                    minimized: root.maximizeLibrary
                    height: root.show4decks ? (root.maximizeLibrary ? 80 : mixer.height / 2) : 0
                    group: "[Channel3]"

                    editMode: root.editDeck

                    states: [
                        State {
                            when: root.maximizeLibrary
                            AnchorChanges { target: deck3; anchors.right: parent.horizontalCenter}
                        }
                    ]

                    Behavior on height {
                        SpringAnimation {
                            id: deck3HeightAnimation
                            duration: 500
                            spring: 2
                            damping: 0.2
                        }
                    }
                }

                Deck {
                    id: deck4

                    anchors {
                        top: deck2.bottom
                        left: mixer.right
                        right: parent.right
                    }

                    minimized: root.maximizeLibrary
                    height: root.show4decks ? (root.maximizeLibrary ? 80 : mixer.height / 2) : 0
                    group: "[Channel4]"

                    editMode: root.editDeck

                    states: [
                        State {
                            when: root.maximizeLibrary
                            AnchorChanges { target: deck4; anchors.left: parent.horizontalCenter}
                        }
                    ]

                    Behavior on height {
                        SpringAnimation {
                            id: deck4HeightAnimation
                            duration: 500
                            spring: 2
                            damping: 0.2
                        }
                    }
                }

                // Skin.SamplerRow {
                //     id: samplers
                //     visible: root.showSamplers
                //     width: parent.width

                //     Skin.FadeBehavior on visible {
                //         fadeTarget: samplers
                //     }
                // }
                // Skin.EffectRow {
                //     id: effects
                //     visible: root.showEffects
                //     width: parent.width

                //     Skin.FadeBehavior on visible {
                //         fadeTarget: effects
                //     }
                // }
                Skin.Library {
                    id: library
                    height: parent.height - y
                    anchors {
                        top: mixer.bottom
                        bottom: parent.bottom
                    }
                    width: parent.width

                    states: [
                        State {
                            when: root.maximizeLibrary && root.show4decks
                            AnchorChanges { target: library; anchors.top: deck4.bottom}
                        },
                        State {
                            when: root.maximizeLibrary && !root.show4decks
                            AnchorChanges { target: library; anchors.top: deck1.bottom}
                        }
                    ]
                }
            }
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
