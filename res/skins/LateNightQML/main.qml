import "../../qml" as Skin
import "LateNightTheme"
import "Deck" as LateNightDeck
import "Waveforms" as LateNightWaveforms
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    property alias editDeck: editDeckButton.checked
    property var focusedDeck: null
    property alias maximizeLibrary: maximizeLibraryButton.checked
    readonly property int fullDeckHeight: 206
    readonly property int minimizedDeckHeight: 80
    readonly property int numDecks: 4
    readonly property int numSamplers: 16
    readonly property bool show4decks: show4DecksButton.checked && show4DecksButton.visible
    property alias showEffects: showEffectsButton.checked
    property alias showSamplers: showSamplersButton.checked

    color: LateNightTheme.backgroundColor
    height: 1008
    minimumHeight: 668
    minimumWidth: 1280
    visible: true
    width: 1792

    Mixxx.ControlProxy {
        group: "[App]"
        key: "num_decks"

        onInitializedChanged: {
            value = root.numDecks;
        }
    }
    Mixxx.ControlProxy {
        group: "[App]"
        key: "num_samplers"

        onInitializedChanged: {
            value = root.numSamplers;
        }
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_waveforms"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_hotcues"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_8_hotcues"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_intro_outro_cues"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_loop_controls"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_beatjump_controls"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_rate_controls"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_rate_control_buttons"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_key_controls"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_vinylcontrol"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_spinnies"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_coverart"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "select_big_spinny_or_cover"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_4effectunits"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_eq_knobs"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_eq_kill_buttons"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_xfader"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_main_head_mixer"
        persist: true
        defaultValue: 1.0
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "equal_4deck_waveforms"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "timing_shift_buttons"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_superknobs"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_sampler_fx"
        persist: true
    }
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

            color: LateNightTheme.toolbarBackgroundColor
            height: 36
            radius: 1
            width: parent.width

            RowLayout {
                anchors.fill: parent

                Skin.Button {
                    id: show4DecksButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "4 Decks"
                    visible: root.height > 515
                }
                Skin.Button {
                    id: maximizeLibraryButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "Library"

                    onCheckedChanged: () => {
                        showMaximizedLibrary.value = this.checked;
                    }

                    Mixxx.ControlProxy {
                        id: showMaximizedLibrary

                        group: "[Skin]"
                        key: "show_maximized_library"

                        onValueChanged: () => {
                            maximizeLibraryButton.checked = this.value;
                        }
                    }
                }
                Skin.Button {
                    id: showEffectsButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "Effects"
                }
                Skin.Button {
                    id: showAuxButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "Aux"
                }
                Skin.Button {
                    id: showSamplersButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "Sampler"
                }
                Item {
                    Layout.fillWidth: true
                }
                Skin.Button {
                    id: editDeckButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    text: "Edit"
                }
                Skin.Button {
                    id: showDevToolsButton

                    activeColor: LateNightTheme.white
                    checkable: true
                    checked: devToolsWindow.visible
                    text: "Develop"

                    onClicked: {
                        if (devToolsWindow.visible)
                            devToolsWindow.close();
                        else
                            devToolsWindow.show();
                    }

                    Skin.DeveloperToolsWindow {
                        id: devToolsWindow

                        height: 480
                        width: 640
                    }
                }
                Skin.Button {
                    id: showPreferencesButton

                    activeColor: LateNightTheme.white
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

            height: parent.height - y
            orientation: Qt.Vertical
            width: parent.width

            handle: Rectangle {
                id: handleDelegate

                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? LateNightTheme.libraryPanelSplitterHandleActive : LateNightTheme.libraryPanelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 3

                clip: true
                color: LateNightTheme.libraryPanelSplitterBackground
                implicitHeight: 4
                implicitWidth: 8

                containmentMask: Item {
                    height: 8
                    width: splitView.width
                    x: (handleDelegate.width - width) / 2
                }

                RowLayout {
                    anchors.centerIn: parent

                    Repeater {
                        model: 3

                        Rectangle {
                            color: handleColor
                            height: handleSize
                            radius: handleSize
                            width: handleSize
                        }
                    }
                }
            }

            LateNightWaveforms.WaveformStack {
                id: waveforms

                SplitView.fillHeight: !library.active
                SplitView.preferredHeight: library.active ? 120 : undefined
                visible: !root.maximizeLibrary
                show4decks: root.show4decks

                Skin.FadeBehavior on visible {
                    fadeTarget: waveforms
                }
            }
            Item {
                id: deckPane

                SplitView.fillHeight: library.active
                SplitView.maximumHeight: library.active ? undefined : mixer.height
                SplitView.minimumHeight: mixer.height
                width: splitView.width

                LateNightDeck.Deck {
                    id: deck1

                    editMode: root.editDeck
                    group: "[Channel1]"
                    height: root.maximizeLibrary ? root.minimizedDeckHeight : root.fullDeckHeight
                    minimized: root.maximizeLibrary

                    Behavior on height {
                        SpringAnimation {
                            id: deck1HeightAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary

                            AnchorChanges {
                                anchors.right: parent.horizontalCenter
                                target: deck1
                            }
                        }
                    ]

                    onToggleFocus: {
                        root.focusedDeck = (root.focusedDeck === deck1) ? null : deck1;
                    }

                    anchors {
                        left: parent.left
                        right: mixer.left
                        top: parent.top
                    }
                }
                Skin.Mixer {
                    id: mixer

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    groups: [deck1.group, deck2.group, deck3.group, deck4.group]
                    width: implicitWidth
                    show4decks: root.show4decks
                    visible: !root.maximizeLibrary

                    Behavior on height {
                        SpringAnimation {
                            id: mixerHeightAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    states: [
                        State {
                            when: root.focusedDeck === deck1 && root.width < 1400 && !root.maximizeLibrary

                            AnchorChanges {
                                anchors.horizontalCenter: parent.right
                                target: mixer
                            }
                            PropertyChanges {
                                target: deck1
                                width: root.width - (mixer.width / 2)
                            }
                        },
                        State {
                            when: root.focusedDeck === deck2 && root.width < 1400 && !root.maximizeLibrary

                            AnchorChanges {
                                anchors.horizontalCenter: parent.left
                                target: mixer
                            }
                            PropertyChanges {
                                target: deck2
                                width: root.width - (mixer.width / 2)
                            }
                        },
                        State {
                            when: (!root.focusedDeck || root.width > 1400) && !root.maximizeLibrary

                            AnchorChanges {
                                anchors.horizontalCenter: parent.horizontalCenter
                                target: mixer
                            }
                            PropertyChanges {
                                target: deck1
                                width: (root.width - mixer.width) / 2
                            }
                            PropertyChanges {
                                target: deck2
                                width: (root.width - mixer.width) / 2
                            }
                        },
                        State {
                            when: root.maximizeLibrary

                            AnchorChanges {
                                anchors.horizontalCenter: parent.horizontalCenter
                                target: mixer
                            }
                            PropertyChanges {
                                target: deck1
                                width: root.width / 2
                            }
                            PropertyChanges {
                                target: deck2
                                width: root.width / 2
                            }
                        }
                    ]
                    transitions: Transition {
                        AnchorAnimation {
                            duration: 200
                        }
                    }
                    Skin.FadeBehavior on visible {
                        fadeTarget: mixer
                    }
                }
                LateNightDeck.Deck {
                    id: deck2

                    editMode: root.editDeck
                    group: "[Channel2]"
                    height: root.maximizeLibrary ? root.minimizedDeckHeight : root.fullDeckHeight
                    minimized: root.maximizeLibrary

                    Behavior on height {
                        SpringAnimation {
                            id: deck2HeightAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary

                            AnchorChanges {
                                anchors.left: parent.horizontalCenter
                                target: deck2
                            }
                        }
                    ]

                    onToggleFocus: {
                        root.focusedDeck = (root.focusedDeck === deck2) ? null : deck2;
                    }

                    anchors {
                        left: mixer.right
                        right: parent.right
                        top: parent.top
                    }
                }
                Loader {
                    id: deck3

                    readonly property string group: "[Channel3]"

                    active: root.show4decks
                    clip: true
                    height: active ? (root.maximizeLibrary ? root.minimizedDeckHeight : root.fullDeckHeight) : 0

                    Behavior on height {
                        SpringAnimation {
                            id: deck3HeightAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    sourceComponent: Component {
                        LateNightDeck.Deck {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            editMode: root.editDeck
                            group: deck3.group
                            minimized: root.maximizeLibrary
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary

                            AnchorChanges {
                                anchors.right: parent.horizontalCenter
                                target: deck3
                            }
                        }
                    ]

                    anchors {
                        left: parent.left
                        right: mixer.left
                        top: deck1.bottom
                    }
                }
                Loader {
                    id: deck4

                    readonly property string group: "[Channel4]"

                    active: root.show4decks
                    clip: true
                    height: active ? (root.maximizeLibrary ? root.minimizedDeckHeight : root.fullDeckHeight) : 0

                    Behavior on height {
                        SpringAnimation {
                            id: deck4HeightAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    sourceComponent: Component {
                        LateNightDeck.Deck {
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            editMode: root.editDeck
                            group: deck4.group
                            minimized: root.maximizeLibrary
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary

                            AnchorChanges {
                                anchors.left: parent.horizontalCenter
                                target: deck4
                            }
                        }
                    ]

                    anchors {
                        left: mixer.right
                        right: parent.right
                        top: deck2.bottom
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
                Loader {
                    id: library

                    active: root.maximizeLibrary || root.height - mixer.height >= 400
                    width: parent.width

                    sourceComponent: Component {
                        Library {
                            anchors.fill: parent
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary && root.show4decks

                            AnchorChanges {
                                anchors.top: deck4.bottom
                                target: library
                            }
                        },
                        State {
                            when: root.maximizeLibrary && !root.show4decks

                            AnchorChanges {
                                anchors.top: deck1.bottom
                                target: library
                            }
                        },
                        State {
                            when: !root.maximizeLibrary && root.height - mixer.height < 400

                            PropertyChanges {
                                target: library
                                visible: false
                            }
                        }
                    ]

                    anchors {
                        bottom: parent.bottom
                        top: mixer.bottom
                    }
                }
            }
        }
    }
    Skin.Settings {
        id: settingsPopup

        height: Math.min(840, parent.height)
        modal: true
        width: Math.min(1400, parent.width)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        Overlay.modal: Rectangle {
            id: overlayModal

            readonly property bool hasHardwareAcceleration: Mixxx.Config.useAcceleration
            property real radius: 12

            anchors.fill: parent
            color: Qt.alpha('#00000010', hasHardwareAcceleration ? 1.0 : 0.6)
        }
    }
}
