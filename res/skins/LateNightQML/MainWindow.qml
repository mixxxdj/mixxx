import "../../qml" as Skin
import "LateNightTheme"
import "Deck" as LateNightDeck
import "Effects" as LateNightEffects
import "MicAux" as LateNightMicAux
import "Mixer" as LateNightMixer
import "Samplers" as LateNightSamplers
import "Toolbar" as LateNightToolbar
import "Waveforms" as LateNightWaveforms
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property ApplicationWindow applicationWindow
    property alias menuBar: nativeApplicationMenuLoader.item

    property alias editDeck: toolbar.editDeck
    property var focusedDeck: null
    readonly property int fullDeckHeight: 206
    property alias maximizeLibrary: toolbar.maximizeLibrary
    readonly property int minimizedDeckHeight: 80
    readonly property int numDecks: 4
    readonly property int numSamplers: 64
    readonly property bool show4decks: toolbar.show4decks
    property alias showEffects: toolbar.showEffects
    property alias showMicAux: toolbar.showMicAux
    readonly property bool showMaximizedDecks: toolbar.showMaximizedDecks
    readonly property bool showMixer: toolbar.showMixer
    property alias showSamplers: toolbar.showSamplers
    readonly property bool showWaveforms: toolbar.showWaveforms

    function focusLegacyLibrarySearch() {
        Qt.callLater(function() {
            if (library.item) {
                library.item.focusSearch();
            }
        });
    }

    Loader {
        id: nativeApplicationMenuLoader

        active: Qt.platform.os === "osx"

        sourceComponent: Skin.MainMenuBar {
            actions: applicationMenuActions
        }
    }
    Skin.ApplicationMenuCommands {
        id: applicationMenuCommands

        applicationWindow: root.applicationWindow

        onShowDeveloperToolsRequested: {
            developerToolsWindow.show();
            developerToolsWindow.raise();
            developerToolsWindow.requestActivate();
        }
    }
    Skin.ApplicationMenuActions {
        id: applicationMenuActions

        applicationWindow: root.applicationWindow
        commands: applicationMenuCommands
        numberOfDecks: root.show4decks ? root.numDecks : 2

        onFocusLibrarySearchRequested: root.focusLegacyLibrarySearch()
    }
    Skin.DeveloperToolsWindow {
        id: developerToolsWindow

        height: 480
        width: 640
    }
    Skin.LibraryScanSummaryDialog {
    }
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
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_waveforms"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_hotcues"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_8_hotcues"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_intro_outro_cues"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_loop_controls"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_beatjump_controls"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_rate_controls"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_rate_control_buttons"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_key_controls"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_vinylcontrol"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_spinnies"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_coverart"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "select_big_spinny_or_cover"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_effectrack"
        persist: true
    }
    Mixxx.SkinControlCreator {
        group: "[Skin]"
        key: "show_4effectunits"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 0.0
        group: "[Skin]"
        key: "show_superknobs"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_eq_knobs"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_eq_kill_buttons"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_xfader"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_main_head_mixer"
        persist: true
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
        // Create this before child controls are constructed so the toolbar
        // toggle and sampler FX assignment proxies can bind reliably.
        defaultValue: 0.0
        group: "[Skin]"
        key: "show_sampler_fx"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "sampler_rows"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "show_4samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: 1.0
        group: "[Skin]"
        key: "show_8samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "show_16samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "show_32samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "show_48samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "show_64samplers"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_1-4"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_1-8"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_9-16"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_17-24"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_25-32"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_33-40"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_41-48"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_49-56"
        persist: true
    }
    Mixxx.SkinControlCreator {
        defaultValue: -1.0
        group: "[Skin]"
        key: "expand_samplers_57-64"
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

        LateNightToolbar.Toolbar {
            id: toolbar

            applicationMenuActions: applicationMenuActions
            show4decksAvailable: root.height > 515
            width: parent.width

            onFocusLibrarySearchRequested: root.focusLegacyLibrarySearch()
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
                show4decks: root.show4decks
                visible: root.showWaveforms && !root.maximizeLibrary

                Skin.FadeBehavior on visible {
                    fadeTarget: waveforms
                }
            }
            Item {
                id: deckPane

                readonly property real basePaneHeight: Math.max(deckRowsHeight, mixer.visible ? mixer.implicitHeight : 0)
                readonly property real deckRowsHeight: root.show4decks ? visibleDeckHeight * 2 : visibleDeckHeight
                readonly property real requiredPaneHeight: basePaneHeight + effectsSection.height + samplersSection.height + micAuxSection.height
                readonly property real visibleDeckHeight: root.maximizeLibrary ? (root.showMaximizedDecks ? root.minimizedDeckHeight : 0) : root.fullDeckHeight

                SplitView.fillHeight: library.active
                SplitView.maximumHeight: library.active ? undefined : requiredPaneHeight
                SplitView.minimumHeight: requiredPaneHeight
                implicitHeight: requiredPaneHeight
                width: splitView.width

                LateNightDeck.Deck {
                    id: deck1

                    editMode: root.editDeck
                    group: "[Channel1]"
                    height: root.maximizeLibrary ? (root.showMaximizedDecks ? root.minimizedDeckHeight : 0) : root.fullDeckHeight
                    minimized: root.maximizeLibrary
                    visible: !root.maximizeLibrary || root.showMaximizedDecks

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
                LateNightMixer.Mixer {
                    id: mixer

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    groups: [deck1.group, deck2.group, deck3.group, deck4.group]
                    height: visible ? implicitHeight : 0
                    show4decks: root.show4decks
                    visible: root.showMixer && !root.maximizeLibrary
                    width: visible ? implicitWidth : 0

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
                    Behavior on width {
                        SpringAnimation {
                            id: mixerWidthAnimation

                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                }
                LateNightDeck.Deck {
                    id: deck2

                    editMode: root.editDeck
                    group: "[Channel2]"
                    height: root.maximizeLibrary ? (root.showMaximizedDecks ? root.minimizedDeckHeight : 0) : root.fullDeckHeight
                    minimized: root.maximizeLibrary
                    visible: !root.maximizeLibrary || root.showMaximizedDecks

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

                    active: root.show4decks && (!root.maximizeLibrary || root.showMaximizedDecks)
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

                    active: root.show4decks && (!root.maximizeLibrary || root.showMaximizedDecks)
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
                Item {
                    id: effectsSection

                    clip: true
                    height: root.showEffects && !root.maximizeLibrary ? effectsRack.implicitHeight : 0
                    opacity: root.showEffects && !root.maximizeLibrary ? 1 : 0
                    visible: height > 0
                    width: parent.width
                    y: deckPane.basePaneHeight
                    z: 2

                    Behavior on height {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.OutCubic
                        }
                    }
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 120
                        }
                    }

                    LateNightEffects.EffectsRack {
                        id: effectsRack

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                    }
                }
                Item {
                    id: samplersSection

                    clip: true
                    height: root.showSamplers && !root.maximizeLibrary ? samplers.implicitHeight : 0
                    opacity: root.showSamplers && !root.maximizeLibrary ? 1 : 0
                    visible: height > 0
                    width: parent.width
                    y: effectsSection.y + effectsSection.height
                    z: 2

                    Behavior on height {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.OutCubic
                        }
                    }
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 120
                        }
                    }

                    LateNightSamplers.SamplersRack {
                        id: samplers

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                    }
                }
                Item {
                    id: micAuxSection

                    clip: true
                    height: root.showMicAux && !root.maximizeLibrary ? micAuxRack.implicitHeight : 0
                    opacity: root.showMicAux && !root.maximizeLibrary ? 1 : 0
                    visible: height > 0
                    width: parent.width
                    y: samplersSection.y + samplersSection.height
                    z: 2

                    Behavior on height {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.OutCubic
                        }
                    }
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 120
                        }
                    }

                    LateNightMicAux.MicAuxRack {
                        id: micAuxRack

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                    }
                }
                Loader {
                    id: library

                    active: true
                    width: parent.width

                    sourceComponent: Component {
                        Library {
                            anchors.fill: parent
                        }
                    }
                    states: [
                        State {
                            when: root.maximizeLibrary && !root.showMaximizedDecks

                            AnchorChanges {
                                anchors.top: parent.top
                                target: library
                            }
                        },
                        State {
                            when: root.maximizeLibrary && root.showMaximizedDecks && root.show4decks

                            AnchorChanges {
                                anchors.top: deck4.bottom
                                target: library
                            }
                        },
                        State {
                            when: root.maximizeLibrary && root.showMaximizedDecks && !root.show4decks

                            AnchorChanges {
                                anchors.top: deck1.bottom
                                target: library
                            }
                        }
                    ]

                    anchors {
                        bottom: parent.bottom
                        top: micAuxSection.bottom
                    }
                }
            }
        }
    }
}
