pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    required property var actions

    signal applicationMenuRequested
    signal menuShortcutTriggered

    readonly property var menuActions: [
        actions.fileLoadDeck1,
        actions.fileLoadDeck2,
        actions.fileLoadDeck3,
        actions.fileLoadDeck4,
        actions.fileQuit,
        actions.libraryRescan,
        actions.librarySearchCurrentView,
        actions.librarySearchTracks,
        actions.libraryCreatePlaylist,
        actions.libraryCreateCrate,
        actions.viewShowMicrophone,
        actions.viewShowVinylControl,
        actions.viewShowPreviewDeck,
        actions.viewShowCoverArt,
        actions.viewShowKeywheel,
        actions.viewMaximizeLibrary,
        actions.viewShowAutoDJ,
        actions.viewFullScreen,
        actions.optionsEnableVinyl1,
        actions.optionsEnableVinyl2,
        actions.optionsEnableVinyl3,
        actions.optionsEnableVinyl4,
        actions.optionsRecordMix,
        actions.optionsEnableLiveBroadcasting,
        actions.optionsEnableKeyboardShortcuts,
        actions.optionsPreferences,
        actions.developerReloadSkin,
        actions.developerTools,
        actions.developerExperimentStats,
        actions.developerBaseStats,
        actions.developerDebugger
    ]

    height: 0
    width: 0

    Connections {
        function onApplicationMenuRequested() {
            root.applicationMenuRequested();
        }

        target: Mixxx.Application
    }
    Shortcut {
        context: Qt.ApplicationShortcut
        enabled: Qt.platform.os !== "osx"
        sequence: "F10"

        onActivated: root.applicationMenuRequested()
    }
    Instantiator {
        model: root.menuActions

        delegate: Shortcut {
            required property var modelData

            context: Qt.ApplicationShortcut
            enabled: Qt.platform.os !== "osx" && modelData.enabled && modelData.shortcut.length > 0
            sequence: modelData.shortcut

            onActivated: {
                modelData.trigger();
                root.menuShortcutTriggered();
            }
        }
    }
}
