pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    property bool show4decks: false
    readonly property var shortcuts: [["FileMenu_LoadDeck1", "Ctrl+O"], ["FileMenu_LoadDeck2", "Ctrl+Shift+O"], ["FileMenu_Quit", "Ctrl+Q"], ["LibraryMenu_Rescan", "Ctrl+Shift+L"], ["LibraryMenu_SearchInCurrentView", "Ctrl+F"], ["LibraryMenu_SearchInAllTracks", "Ctrl+Shift+F"], ["LibraryMenu_NewPlaylist", "Ctrl+N"], ["LibraryMenu_NewCrate", "Ctrl+Shift+N"], ["ViewMenu_ShowMicrophone", "Ctrl+2"], ["ViewMenu_ShowVinylControl", "Ctrl+3"], ["ViewMenu_ShowPreviewDeck", "Ctrl+4"], ["ViewMenu_ShowCoverArt", "Ctrl+6"], ["ViewMenu_ShowKeywheel", "F12"], ["ViewMenu_MaximizeLibrary", "Space"], ["ViewMenu_ShowAutoDJ", "Ctrl+9"], ["ViewMenu_FullScreen", "F11"], ["OptionsMenu_EnableVinyl1", "Ctrl+T"], ["OptionsMenu_EnableVinyl2", "Ctrl+Y"], ["OptionsMenu_EnableVinyl3", "Ctrl+U"], ["OptionsMenu_EnableVinyl4", "Ctrl+I"], ["OptionsMenu_RecordMix", "Ctrl+R"], ["OptionsMenu_EnableLiveBroadcasting", "Ctrl+L"], ["OptionsMenu_EnableShortcuts", "Ctrl+`"], ["OptionsMenu_Preferences", "Ctrl+P"], ["OptionsMenu_ReloadSkin", "Ctrl+Shift+R"], ["OptionsMenu_DeveloperTools", "Ctrl+Shift+T"], ["OptionsMenu_DeveloperStatsExperiment", "Ctrl+Shift+E"], ["OptionsMenu_DeveloperStatsBase", "Ctrl+Shift+B"], ["DeveloperMenu_EnableDebugger", "Ctrl+Shift+D"]]

    signal applicationMenuRequested
    signal shortcutTriggered(string command)

    function shortcutEnabled(command) {
        switch (command) {
        case "OptionsMenu_EnableVinyl3":
        case "OptionsMenu_EnableVinyl4":
            return root.show4decks;
        case "OptionsMenu_ReloadSkin":
        case "OptionsMenu_DeveloperTools":
        case "OptionsMenu_DeveloperStatsExperiment":
        case "OptionsMenu_DeveloperStatsBase":
        case "DeveloperMenu_EnableDebugger":
            return Mixxx.Application.developerMode;
        default:
            return true;
        }
    }

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
        model: root.shortcuts

        delegate: Shortcut {
            required property var modelData

            context: Qt.ApplicationShortcut
            enabled: Qt.platform.os !== "osx" && root.shortcutEnabled(modelData[0])
            sequence: Mixxx.Application.menuShortcut(modelData[0], modelData[1])

            onActivated: {
                root.shortcutTriggered(modelData[0]);
            }
        }
    }
}
