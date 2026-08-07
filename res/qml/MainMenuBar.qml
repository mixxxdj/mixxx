pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Window

MenuBar {
    id: root

    required property ApplicationWindow applicationWindow
    required property var commands
    property int numberOfDecks: 4

    signal focusLibrarySearchRequested

    visible: Qt.platform.os === "osx"

    Menu {
        title: qsTr("&File")

        Action {
            shortcut: Mixxx.Application.menuShortcut("FileMenu_LoadDeck1", "Ctrl+O")
            text: qsTr("Load Track to Deck &1")

            onTriggered: root.commands.loadTrackToDeck(1)
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("FileMenu_LoadDeck2", "Ctrl+Shift+O")
            text: qsTr("Load Track to Deck &2")

            onTriggered: root.commands.loadTrackToDeck(2)
        }
        Action {
            enabled: root.numberOfDecks >= 3
            text: qsTr("Load Track to Deck &3")

            onTriggered: root.commands.loadTrackToDeck(3)
        }
        Action {
            enabled: root.numberOfDecks >= 4
            text: qsTr("Load Track to Deck &4")

            onTriggered: root.commands.loadTrackToDeck(4)
        }
        MenuSeparator {
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("FileMenu_Quit", "Ctrl+Q")
            text: qsTr("E&xit")

            onTriggered: Qt.quit()
        }
    }
    Menu {
        title: qsTr("&Library")

        Action {
            enabled: !Mixxx.Library.libraryScanActive
            shortcut: Mixxx.Application.menuShortcut("LibraryMenu_Rescan", "Ctrl+Shift+L")
            text: qsTr("&Rescan Library")

            onTriggered: Mixxx.Library.rescanLibrary()
        }
        Action {
            enabled: Mixxx.Library.enginePrimeExportAvailable
            text: qsTr("E&xport Library to Engine DJ")

            onTriggered: Mixxx.Library.exportLibrary()
        }
        MenuSeparator {
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("LibraryMenu_SearchInCurrentView", "Ctrl+F")
            text: qsTr("Search in Current View...")

            onTriggered: {
                Mixxx.Library.searchInCurrentView();
                root.focusLibrarySearchRequested();
            }
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("LibraryMenu_SearchInAllTracks", "Ctrl+Shift+F")
            text: qsTr("Search in Tracks Library...")

            onTriggered: {
                Mixxx.Library.searchInTracksLibrary();
                root.focusLibrarySearchRequested();
            }
        }
        MenuSeparator {
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("LibraryMenu_NewPlaylist", "Ctrl+N")
            text: qsTr("Create &New Playlist")

            onTriggered: Mixxx.Library.createPlaylist()
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("LibraryMenu_NewCrate", "Ctrl+Shift+N")
            text: qsTr("Create New &Crate")

            onTriggered: Mixxx.Library.createCrate()
        }
    }
    Menu {
        title: qsTr("&View") + "\u200c"

        Action {
            checkable: true
            checked: showMicrophonesControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowMicrophone", "Ctrl+2")
            text: qsTr("Show Microphone Section")

            onTriggered: showMicrophonesControl.value = showMicrophonesControl.value > 0 ? 0.0 : 1.0
        }
        Action {
            checkable: true
            checked: showVinylControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowVinylControl", "Ctrl+3")
            text: qsTr("Show Vinyl Control Section")

            onTriggered: showVinylControl.value = showVinylControl.value > 0 ? 0.0 : 1.0
        }
        Action {
            checkable: true
            checked: showPreviewDecksControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowPreviewDeck", "Ctrl+4")
            text: qsTr("Show Preview Deck")

            onTriggered: showPreviewDecksControl.value = showPreviewDecksControl.value > 0 ? 0.0 : 1.0
        }
        Action {
            checkable: true
            checked: showLibraryCoverArtControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowCoverArt", "Ctrl+6")
            text: qsTr("Show Cover Art")

            onTriggered: showLibraryCoverArtControl.value = showLibraryCoverArtControl.value > 0 ? 0.0 : 1.0
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowKeywheel", "F12")
            text: qsTr("Show Keywheel")

            onTriggered: root.commands.showKeywheel()
        }
        Action {
            checkable: true
            checked: showMaximizedLibraryControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_MaximizeLibrary", "Space")
            text: qsTr("Maximize Library")

            onTriggered: showMaximizedLibraryControl.value = showMaximizedLibraryControl.value > 0 ? 0.0 : 1.0
        }
        MenuSeparator {
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("ViewMenu_ShowAutoDJ", "Ctrl+9")
            text: qsTr("Show Auto DJ")

            onTriggered: Mixxx.Library.showAutoDJ()
        }
        Action {
            checkable: true
            checked: root.applicationWindow.visibility === Window.FullScreen
            shortcut: Qt.platform.os === "osx" ? "Ctrl+Meta+F" : "F11"
            text: qsTr("&Full Screen")

            onTriggered: {
                root.commands.toggleFullScreen();
            }
        }
    }
    Menu {
        title: qsTr("&Options")

        Menu {
            title: qsTr("Vinyl Control")

            Action {
                checkable: true
                checked: vinylDeck1Control.value > 0
                shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableVinyl1", "Ctrl+T")
                text: qsTr("Enable Vinyl Control 1")

                onTriggered: vinylDeck1Control.value = vinylDeck1Control.value > 0 ? 0.0 : 1.0
            }
            Action {
                checkable: true
                checked: vinylDeck2Control.value > 0
                shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableVinyl2", "Ctrl+Y")
                text: qsTr("Enable Vinyl Control 2")

                onTriggered: vinylDeck2Control.value = vinylDeck2Control.value > 0 ? 0.0 : 1.0
            }
            Action {
                checkable: true
                checked: vinylDeck3Control.value > 0
                shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableVinyl3", "Ctrl+U")
                text: qsTr("Enable Vinyl Control 3")

                onTriggered: vinylDeck3Control.value = vinylDeck3Control.value > 0 ? 0.0 : 1.0
            }
            Action {
                checkable: true
                checked: vinylDeck4Control.value > 0
                shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableVinyl4", "Ctrl+I")
                text: qsTr("Enable Vinyl Control 4")

                onTriggered: vinylDeck4Control.value = vinylDeck4Control.value > 0 ? 0.0 : 1.0
            }
        }
        MenuSeparator {
        }
        Action {
            checkable: true
            checked: recordingStatusControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_RecordMix", "Ctrl+R")
            text: qsTr("&Record Mix")

            onTriggered: recordingToggleControl.trigger()
        }
        Action {
            checkable: true
            checked: broadcastEnabledControl.value > 0
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableLiveBroadcasting", "Ctrl+L")
            text: qsTr("Enable Live &Broadcasting")

            onTriggered: broadcastEnabledControl.value = broadcastEnabledControl.value > 0 ? 0.0 : 1.0
        }
        Action {
            checkable: true
            checked: Mixxx.Application.keyboardShortcutsEnabled
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_EnableShortcuts", "Ctrl+`")
            text: qsTr("Enable &Keyboard Shortcuts")

            onTriggered: Mixxx.Application.keyboardShortcutsEnabled = !Mixxx.Application.keyboardShortcutsEnabled
        }
        MenuSeparator {
        }
        Action {
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_Preferences", "Ctrl+,")
            text: qsTr("&Preferences")

            onTriggered: Mixxx.PreferencesDialog.show()
        }
    }
    Menu {
        title: qsTr("&Developer")
        visible: Mixxx.Application.developerMode

        Action {
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_ReloadSkin", "Ctrl+Shift+R")
            text: qsTr("&Reload Skin")

            onTriggered: Mixxx.Application.reloadSkin()
        }

        Action {
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_DeveloperTools", "Ctrl+Shift+T")
            text: qsTr("Developer &Tools")

            onTriggered: root.commands.showDeveloperToolsRequested()
        }
        Action {
            checkable: true
            checked: Mixxx.Application.experimentStatsEnabled
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_DeveloperStatsExperiment", "Ctrl+Shift+E")
            text: qsTr("Stats: &Experiment Bucket")

            onTriggered: Mixxx.Application.setExperimentStatsEnabled(!Mixxx.Application.experimentStatsEnabled)
        }
        Action {
            checkable: true
            checked: Mixxx.Application.baseStatsEnabled
            shortcut: Mixxx.Application.menuShortcut("OptionsMenu_DeveloperStatsBase", "Ctrl+Shift+B")
            text: qsTr("Stats: &Base Bucket")

            onTriggered: Mixxx.Application.setBaseStatsEnabled(!Mixxx.Application.baseStatsEnabled)
        }
        Action {
            checkable: true
            checked: Mixxx.Application.debuggerEnabled
            shortcut: Mixxx.Application.menuShortcut("DeveloperMenu_EnableDebugger", "Ctrl+Shift+D")
            text: qsTr("Deb&ugger Enabled")

            onTriggered: Mixxx.Application.debuggerEnabled = !Mixxx.Application.debuggerEnabled
        }
    }
    Menu {
        title: qsTr("&Help")

        Action {
            text: qsTr("&Community Support")

            onTriggered: Qt.openUrlExternally("https://www.mixxx.org/support/")
        }
        Action {
            text: qsTr("&User Manual")

            onTriggered: Qt.openUrlExternally("https://manual.mixxx.org/2.7/")
        }
        Action {
            text: qsTr("&Keyboard Shortcuts")

            onTriggered: Qt.openUrlExternally("https://manual.mixxx.org/2.7/chapters/controlling_mixxx.html#using-a-keyboard")
        }
        Action {
            text: qsTr("&Settings directory")

            onTriggered: Qt.openUrlExternally(Mixxx.Application.settingsDirectoryUrl)
        }
        Action {
            text: qsTr("&Translate This Application")

            onTriggered: Qt.openUrlExternally("https://explore.transifex.com/mixxx-dj-software/")
        }
        MenuSeparator {
        }
        Action {
            text: qsTr("&About")

            onTriggered: root.commands.showAbout()
        }
    }
    Mixxx.ControlProxy {
        id: showMicrophonesControl

        group: "[Skin]"
        key: "show_microphones"
    }
    Mixxx.ControlProxy {
        id: showVinylControl

        group: "[Skin]"
        key: "show_vinylcontrol"
    }
    Mixxx.ControlProxy {
        id: showPreviewDecksControl

        group: "[Skin]"
        key: "show_preview_decks"
    }
    Mixxx.ControlProxy {
        id: showLibraryCoverArtControl

        group: "[Skin]"
        key: "show_library_coverart"
    }
    Mixxx.ControlProxy {
        id: showMaximizedLibraryControl

        group: "[Skin]"
        key: "show_maximized_library"
    }
    Mixxx.ControlProxy {
        id: vinylDeck1Control

        group: "[Channel1]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck2Control

        group: "[Channel2]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck3Control

        group: "[Channel3]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck4Control

        group: "[Channel4]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: recordingStatusControl

        group: "[Recording]"
        key: "status"
    }
    Mixxx.ControlProxy {
        id: recordingToggleControl

        group: "[Recording]"
        key: "toggle_recording"
    }
    Mixxx.ControlProxy {
        id: broadcastEnabledControl

        group: "[Shoutcast]"
        key: "enabled"
    }
}
