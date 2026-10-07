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
import QtQuick.Templates as T
import QtQuick.Window

Item {
    id: root

    required property ApplicationWindow applicationWindow
    property alias menuBar: nativeApplicationMenuLoader.item

    readonly property int activeDeckState: layoutState.effectiveDeckSize
    readonly property int activeDeckHeight: activeDeckState === 0 ? LateNightTheme.miniDeckHeight : (activeDeckState === 1 ? LateNightTheme.compactDeckHeight : LateNightTheme.fullDeckHeight)
    property alias editDeck: toolbar.editDeck
    property var focusedDeck: null
    property alias maximizeLibrary: toolbar.maximizeLibrary
    property bool layoutMaximizedLibrary: false
    property bool bigLibraryTransition: false
    property bool bigLibraryFadeIn: false
    property bool deckModeTransition: false
    property bool bigLibraryEffectsRestoreActive: false
    readonly property int bigLibraryFadeDuration: 60
    readonly property bool bigLibraryContentReady: deck1.contentReady && deck2.contentReady
            && (!deck3.shouldShow || (deck3.status === Loader.Ready && (deck3.item as LateNightDeck.Deck).contentReady))
            && (!deck4.shouldShow || (deck4.status === Loader.Ready && (deck4.item as LateNightDeck.Deck).contentReady))
            && (!showSamplers || samplers.contentReady)
            && library.status === Loader.Ready
    readonly property bool bigLibraryAnimationsReady: !waveformsMinimumHeightAnimation.running
            && !waveformsPreferredHeightAnimation.running
            && !waveformsOpacityAnimation.running
            && !deckHeightAnimation.running
            && !deckRowExpansionAnimation.running
            && !deckSideMarginLayoutAnimation.running
            && !deck1AnchorAnimation.running
            && !compactVuSlotWidthAnimation.running
            && !mixerAnchorAnimation.running
            && !mixerLibraryWidthAnimation.running
            && !deck2AnchorAnimation.running
            && !deck3AnchorAnimation.running
            && !deck3OpacityAnimation.running
            && !deck4AnchorAnimation.running
            && !deck4OpacityAnimation.running
            && !libraryAnchorAnimation.running
    readonly property bool restoringFromBigLibrary: bigLibraryTransition && !maximizeLibrary
    readonly property int bigLibraryTransitionDuration: restoringFromBigLibrary ? 80 : 100
    readonly property int deckModeTransitionDuration: 100
    readonly property int normalDeckState: layoutState.normalizedSavedDeckSize
    readonly property int numDecks: 4
    readonly property int numSamplers: 64
    readonly property bool show4decks: toolbar.show4decks
    property alias showEffects: toolbar.showEffects
    readonly property bool showCompactVuMeters: layoutState.showCompactVuMeters
    readonly property bool showDeckArea: layoutState.showDeckArea
    property alias showMicAux: toolbar.showMicAux
    readonly property bool showMaximizedDecks: toolbar.showMaximizedDecks
    readonly property bool showMixer: toolbar.showMixer
    property alias showSamplers: toolbar.showSamplers
    readonly property bool showWaveforms: toolbar.showWaveforms

    onMaximizeLibraryChanged: {
        bigLibraryReadinessTimer.stop();
        bigLibraryFadeInTimer.stop();
        bigLibraryEffectsRestoreTimer.stop();
        bigLibraryTransition = true;
        bigLibraryFadeIn = false;
        bigLibraryEffectsRestoreActive = !maximizeLibrary;
        deckModeTransition = false;
        deckModeTransitionTimer.stop();
        layoutMaximizedLibrary = maximizeLibrary;
        bigLibraryReadinessTimer.start();
    }
    onNormalDeckStateChanged: {
        if (bigLibraryTransition || maximizeLibrary || showMixer)
            return;

        deckModeTransition = true;
        deckModeTransitionTimer.restart();
    }
    onShowMixerChanged: {
        if (showMixer || bigLibraryTransition || maximizeLibrary || normalDeckState !== 1)
            return;

        deckModeTransition = true;
        deckModeTransitionTimer.restart();
    }
    Component.onCompleted: {
        if (!bigLibraryTransition)
            layoutMaximizedLibrary = maximizeLibrary;
    }

    function finishBigLibraryTransitionWhenReady() {
        if (!bigLibraryTransition || !bigLibraryContentReady || !bigLibraryAnimationsReady)
            return;

        bigLibraryReadinessTimer.stop();
        bigLibraryFadeIn = true;
        bigLibraryTransition = false;
        bigLibraryFadeInTimer.restart();
        if (bigLibraryEffectsRestoreActive)
            bigLibraryEffectsRestoreTimer.restart();
    }

    Timer {
        id: bigLibraryReadinessTimer

        interval: 16
        repeat: true
        onTriggered: root.finishBigLibraryTransitionWhenReady()
    }
    Timer {
        id: bigLibraryFadeInTimer

        interval: root.bigLibraryFadeDuration + 16
        onTriggered: root.bigLibraryFadeIn = false
    }
    Timer {
        id: bigLibraryEffectsRestoreTimer

        interval: 180
        onTriggered: root.bigLibraryEffectsRestoreActive = false
    }
    Timer {
        id: deckModeTransitionTimer

        interval: root.deckModeTransitionDuration + 10
        onTriggered: root.deckModeTransition = false
    }

    SkinControlBootstrap {
        id: skinControlBootstrap
    }

    // Declare the compact-meter setting before LayoutState so its initial
    // value is available when the effective layout is derived.
    Mixxx.ControlProxy {
        id: showCompactVuMetersProxy

        group: "[Skin]"
        key: "show_vumeters_compact"
    }

    LayoutState {
        id: layoutState

        maximizeLibrary: root.layoutMaximizedLibrary
        mixerVisible: root.showMixer
        savedDeckSize: toolbar.deckSizeWithoutMixer
        show4decks: root.show4decks
        showCompactVuMetersSetting: showCompactVuMetersProxy.value > 0
        showMaximizedDecks: root.showMaximizedDecks
    }

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
    Column {
        id: content

        anchors.fill: parent

        move: Transition {
            enabled: LateNightTheme.layoutAnimationsEnabled

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

                readonly property bool pressed: splitView.resizing || T.SplitHandle.pressed

                clip: true
                color: LateNightTheme.libraryPanelSplitterBackground
                implicitHeight: 9
                implicitWidth: 8

                containmentMask: Item {
                    height: 12
                    width: splitView.width
                    x: (handleDelegate.width - width) / 2
                }

                Image {
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    source: handleDelegate.pressed
                            ? LateNightTheme.assetWaveformSplitterHandlePressed
                            : LateNightTheme.assetWaveformSplitterHandle
                }
            }

            LateNightWaveforms.WaveformStack {
                id: waveforms

                readonly property bool shouldShow: root.showWaveforms && !root.layoutMaximizedLibrary
                property real paneMinimumHeight: shouldShow ? minimumContentHeight : 0
                property real panePreferredHeight: shouldShow ? Math.max(120, minimumContentHeight) : 0

                SplitView.fillHeight: !library.active
                SplitView.minimumHeight: paneMinimumHeight
                implicitHeight: panePreferredHeight
                layoutTransitioning: root.bigLibraryTransition
                splitterResizing: splitView.resizing
                show4decks: root.show4decks
                visible: panePreferredHeight > 0
                opacity: shouldShow && !root.bigLibraryTransition ? 1 : 0
                clip: true

                Behavior on paneMinimumHeight {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    NumberAnimation {
                        id: waveformsMinimumHeightAnimation

                        duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 180
                        easing.type: Easing.OutCubic
                    }
                }
                Behavior on panePreferredHeight {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    NumberAnimation {
                        id: waveformsPreferredHeightAnimation

                        duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 180
                        easing.type: Easing.OutCubic
                    }
                }
                Behavior on opacity {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    NumberAnimation {
                        id: waveformsOpacityAnimation

                        duration: root.bigLibraryTransition || root.bigLibraryFadeIn ? root.bigLibraryFadeDuration : 120
                    }
                }
            }
            Rectangle {
                id: deckPane

                color: LateNightTheme.layoutGutterColor
                property real deckRowExpansion: root.show4decks ? 1 : 0
                readonly property real deckRowCount: 1 + Math.max(0, deckRowExpansion)
                readonly property real basePaneHeight: Math.max(deckRowsHeight, mixerLayoutVisible ? mixer.implicitHeight + LateNightTheme.deckRowGutter : 0)
                readonly property real deckRowHeight: visibleDeckHeight > 0
                        ? (deckStackHeight - LateNightTheme.deckRowGutter * (deckRowCount - 1)) / deckRowCount
                        : 0
                readonly property real deckRowsHeight: visibleDeckHeight > 0
                        ? (visibleDeckHeight + LateNightTheme.deckRowGutter) * deckRowCount
                        : 0
                property real deckSideMargin: root.showMixer && !root.layoutMaximizedLibrary ? LateNightTheme.deckMixerGutter : 2
                readonly property real deckStackHeight: basePaneHeight - LateNightTheme.deckRowGutter
                readonly property bool mixerLayoutVisible: root.showMixer && !root.layoutMaximizedLibrary
                readonly property real requiredPaneHeight: basePaneHeight + effectsSection.height + samplersSection.height + micAuxSection.height
                property real visibleDeckHeight: root.layoutMaximizedLibrary ? (root.showMaximizedDecks ? LateNightTheme.miniDeckHeight : 0) : root.activeDeckHeight

                SplitView.fillHeight: library.active
                SplitView.maximumHeight: library.active ? undefined : requiredPaneHeight
                SplitView.minimumHeight: requiredPaneHeight
                implicitHeight: requiredPaneHeight
                width: splitView.width

                Behavior on visibleDeckHeight {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    NumberAnimation {
                        id: deckHeightAnimation

                        duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration
                                : root.deckModeTransition ? root.deckModeTransitionDuration : 250
                        easing.type: Easing.OutCubic
                    }
                }
                Behavior on deckRowExpansion {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    NumberAnimation {
                        id: deckRowExpansionAnimation

                        duration: 120
                        easing.type: Easing.OutCubic
                    }
                }
                Behavior on deckSideMargin {
                    enabled: LateNightTheme.layoutAnimationsEnabled

                    animation: root.bigLibraryTransition ? deckSideMarginLayoutAnimation : deckSideMarginSpringAnimation
                }
                NumberAnimation {
                    id: deckSideMarginLayoutAnimation

                    duration: root.bigLibraryTransitionDuration
                    easing.type: Easing.OutCubic
                }
                SpringAnimation {
                    id: deckSideMarginSpringAnimation

                    damping: 0.2
                    duration: 500
                    spring: 2
                }

                Item {
                    id: deckFirstRowBottom

                    height: 0
                    y: deckPane.deckRowHeight
                }
                Item {
                    id: deckStackBottom

                    height: 0
                    y: deckPane.deckStackHeight
                }
                LateNightDeck.Deck {
                    id: deck1

                    deckState: root.layoutMaximizedLibrary ? LateNightDeck.Deck.Mini : root.activeDeckState
                    editMode: root.editDeck
                    group: "[Channel1]"
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: !root.layoutMaximizedLibrary || root.showMaximizedDecks
                    onToggleFocus: {
                        root.focusedDeck = (root.focusedDeck === deck1) ? null : deck1;
                    }

                    anchors {
                        bottom: deckFirstRowBottom.top
                        left: parent.left
                        right: mixer.left
                        rightMargin: Math.max(2, deckPane.deckSideMargin)
                        top: parent.top
                    }

                    states: [
                        State {
                            when: root.showCompactVuMeters && !root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.right: compactVuSlot.left
                                target: deck1
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.right: parent.horizontalCenter
                                target: deck1
                            }
                        }
                    ]
                    transitions: Transition {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: deck1AnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 0
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: deck1OpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }
                }
                Item {
                    id: compactVuSlot

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    height: root.showCompactVuMeters ? deckPane.deckStackHeight : 0
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: root.showCompactVuMeters || compactVuSlotWidthAnimation.running
                    width: root.showCompactVuMeters ? LateNightTheme.compactVuSlotWidth : 0
                    z: 10

                    Behavior on width {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: compactVuSlotWidthAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration
                                    : root.deckModeTransition ? root.deckModeTransitionDuration : 250
                            easing.type: Easing.OutCubic
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: compactVuSlotOpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: LateNightTheme.compactVuGutterColor
                    }
                    LateNightMixer.CompactCenterVuMeters {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        show4decks: root.show4decks
                    }
                }
                LateNightMixer.Mixer {
                    id: mixer

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    clip: true
                    groups: [deck1.group, deck2.group, deck3.group, deck4.group]
                    height: deckPane.mixerLayoutVisible || mixer.width > 0 ? deckPane.deckStackHeight : 0
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    show4decks: root.show4decks
                    visible: deckPane.mixerLayoutVisible || mixer.width > 0
                    width: deckPane.mixerLayoutVisible ? implicitWidth : 0

                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: mixerOpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }

                    states: [
                        State {
                            when: root.showMixer && root.focusedDeck === deck1 && root.width < 1400 && !root.layoutMaximizedLibrary

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
                            when: root.showMixer && root.focusedDeck === deck2 && root.width < 1400 && !root.layoutMaximizedLibrary

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
                            when: root.showMixer && (!root.focusedDeck || root.width > 1400) && !root.layoutMaximizedLibrary

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
                            when: root.layoutMaximizedLibrary

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
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: mixerAnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 200
                        }
                    }
                    Behavior on width {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        animation: root.bigLibraryTransition ? mixerLibraryWidthAnimation : mixerWidthAnimation
                    }
                    NumberAnimation {
                        id: mixerLibraryWidthAnimation

                        duration: root.bigLibraryTransitionDuration
                        easing.type: Easing.OutCubic
                    }
                    SpringAnimation {
                        id: mixerWidthAnimation

                        damping: 0.2
                        duration: 500
                        spring: 2
                    }
                }
                LateNightDeck.Deck {
                    id: deck2

                    deckState: root.layoutMaximizedLibrary ? LateNightDeck.Deck.Mini : root.activeDeckState
                    editMode: root.editDeck
                    group: "[Channel2]"
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: !root.layoutMaximizedLibrary || root.showMaximizedDecks
                    onToggleFocus: {
                        root.focusedDeck = (root.focusedDeck === deck2) ? null : deck2;
                    }

                    anchors {
                        bottom: deckFirstRowBottom.top
                        left: mixer.right
                        leftMargin: Math.max(2, deckPane.deckSideMargin)
                        right: parent.right
                        top: parent.top
                    }

                    states: [
                        State {
                            when: root.showCompactVuMeters && !root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.left: compactVuSlot.right
                                target: deck2
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.left: parent.horizontalCenter
                                target: deck2
                            }
                        }
                    ]
                    transitions: Transition {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: deck2AnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 0
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: deck2OpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }
                }
                Loader {
                    id: deck3

                    readonly property string group: "[Channel3]"
                    readonly property bool shouldShow: root.show4decks && (!root.layoutMaximizedLibrary || root.showMaximizedDecks)

                    active: shouldShow || opacity > 0
                    clip: true
                    opacity: shouldShow && !root.bigLibraryTransition && !root.deckModeTransition ? 1 : 0
                    sourceComponent: Component {
                        LateNightDeck.Deck {
                            anchors.fill: parent
                            deckState: root.layoutMaximizedLibrary ? LateNightDeck.Deck.Mini : root.activeDeckState
                            editMode: root.editDeck
                            group: deck3.group
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: deck3OpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }
                    states: [
                        State {
                            when: root.showCompactVuMeters && !root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.right: compactVuSlot.left
                                target: deck3
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.right: parent.horizontalCenter
                                target: deck3
                            }
                        }
                    ]
                    transitions: Transition {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: deck3AnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 0
                        }
                    }

                    anchors {
                        bottom: deckStackBottom.top
                        left: parent.left
                        right: mixer.left
                        rightMargin: Math.max(2, deckPane.deckSideMargin)
                        top: deckFirstRowBottom.bottom
                        topMargin: LateNightTheme.deckRowGutter * Math.max(0, deckPane.deckRowExpansion)
                    }
                }
                Loader {
                    id: deck4

                    readonly property string group: "[Channel4]"
                    readonly property bool shouldShow: root.show4decks && (!root.layoutMaximizedLibrary || root.showMaximizedDecks)

                    active: shouldShow || opacity > 0
                    clip: true
                    opacity: shouldShow && !root.bigLibraryTransition && !root.deckModeTransition ? 1 : 0
                    sourceComponent: Component {
                        LateNightDeck.Deck {
                            anchors.fill: parent
                            deckState: root.layoutMaximizedLibrary ? LateNightDeck.Deck.Mini : root.activeDeckState
                            editMode: root.editDeck
                            group: deck4.group
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: deck4OpacityAnimation

                            duration: root.deckModeTransition && !root.bigLibraryTransition ? 0 : root.bigLibraryFadeDuration
                        }
                    }
                    states: [
                        State {
                            when: root.showCompactVuMeters && !root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.left: compactVuSlot.right
                                target: deck4
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary

                            AnchorChanges {
                                anchors.left: parent.horizontalCenter
                                target: deck4
                            }
                        }
                    ]
                    transitions: Transition {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: deck4AnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 0
                        }
                    }

                    anchors {
                        bottom: deckStackBottom.top
                        left: mixer.right
                        leftMargin: Math.max(2, deckPane.deckSideMargin)
                        right: parent.right
                        top: deckFirstRowBottom.bottom
                        topMargin: LateNightTheme.deckRowGutter * Math.max(0, deckPane.deckRowExpansion)
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
                    height: root.showEffects && !root.layoutMaximizedLibrary ? effectsRack.implicitHeight : 0
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: height > 0
                    width: parent.width
                    y: deckPane.basePaneHeight
                    z: 2

                    Behavior on height {
                        enabled: LateNightTheme.layoutAnimationsEnabled && (!root.bigLibraryTransition || root.bigLibraryEffectsRestoreActive)

                        SpringAnimation {
                            damping: 0.2
                            duration: root.bigLibraryEffectsRestoreActive ? 180 : 500
                            spring: 2
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: effectsSectionOpacityAnimation

                            duration: 60
                        }
                    }
                    LateNightEffects.EffectsRack {
                        id: effectsRack

                        anchors.fill: parent
                        leftUnitEnd: deckPane.mixerLayoutVisible || mixer.width > 0 || root.showCompactVuMeters
                                ? Math.round((effectsRack.width - effectsRack.unitSpacing) / 2)
                                : deck1.x + deck1.width
                        rightUnitStart: deckPane.mixerLayoutVisible || mixer.width > 0 || root.showCompactVuMeters
                                ? Math.round((effectsRack.width + effectsRack.unitSpacing) / 2)
                                : deck2.x
                    }
                }
                Item {
                    id: samplersSection

                    clip: true
                    height: root.showSamplers && !root.layoutMaximizedLibrary ? samplers.implicitHeight : 0
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: height > 0
                    width: parent.width
                    y: effectsSection.y + effectsSection.height
                    z: 2

                    Behavior on height {
                        enabled: LateNightTheme.layoutAnimationsEnabled && !root.bigLibraryTransition

                        SpringAnimation {
                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: samplersSectionOpacityAnimation

                            duration: 60
                        }
                    }
                    LateNightSamplers.SamplersRack {
                        id: samplers

                        anchors.fill: parent
                    }
                }
                Item {
                    id: micAuxSection

                    clip: true
                    height: root.showMicAux && !root.layoutMaximizedLibrary ? micAuxRack.implicitHeight : 0
                    opacity: root.bigLibraryTransition || root.deckModeTransition ? 0 : 1
                    visible: height > 0
                    width: parent.width
                    y: samplersSection.y + samplersSection.height
                    z: 2

                    Behavior on height {
                        enabled: LateNightTheme.layoutAnimationsEnabled && !root.bigLibraryTransition

                        SpringAnimation {
                            damping: 0.2
                            duration: 500
                            spring: 2
                        }
                    }
                    Behavior on opacity {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        NumberAnimation {
                            id: micAuxSectionOpacityAnimation

                            duration: 60
                        }
                    }
                    LateNightMicAux.MicAuxRack {
                        id: micAuxRack

                        anchors.fill: parent
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
                            when: root.layoutMaximizedLibrary && !root.showMaximizedDecks

                            AnchorChanges {
                                anchors.top: parent.top
                                target: library
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary && root.showMaximizedDecks && root.show4decks

                            AnchorChanges {
                                anchors.top: deck4.bottom
                                target: library
                            }
                        },
                        State {
                            when: root.layoutMaximizedLibrary && root.showMaximizedDecks && !root.show4decks

                            AnchorChanges {
                                anchors.top: deck1.bottom
                                target: library
                            }
                        }
                    ]
                    transitions: Transition {
                        enabled: LateNightTheme.layoutAnimationsEnabled

                        AnchorAnimation {
                            id: libraryAnchorAnimation

                            duration: root.bigLibraryTransition ? root.bigLibraryTransitionDuration : 100
                            easing.type: Easing.OutCubic
                        }
                    }

                    anchors {
                        bottom: parent.bottom
                        top: micAuxSection.bottom
                    }
                }
            }
        }
    }
}
