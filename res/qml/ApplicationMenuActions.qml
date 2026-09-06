import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Window

Item {
    id: root

    required property ApplicationWindow applicationWindow
    required property ApplicationMenuCommands commands
    property alias developerBaseStats: developerBaseStatsAction
    property alias developerDebugger: developerDebuggerAction
    property alias developerExperimentStats: developerExperimentStatsAction
    property alias developerReloadSkin: developerReloadSkinAction
    property alias developerTools: developerToolsAction
    readonly property string externalLinkSuffix: Qt.platform.os === "osx" ? "" : " \u2197"
    property alias fileLoadDeck1: fileLoadDeck1Action
    property alias fileLoadDeck2: fileLoadDeck2Action
    property alias fileLoadDeck3: fileLoadDeck3Action
    property alias fileLoadDeck4: fileLoadDeck4Action
    property alias fileQuit: fileQuitAction
    property alias helpAbout: helpAboutAction
    property alias helpCommunitySupport: helpCommunitySupportAction
    property alias helpKeyboardShortcuts: helpKeyboardShortcutsAction
    property alias helpSettingsDirectory: helpSettingsDirectoryAction
    property alias helpTranslate: helpTranslateAction
    property alias helpUserManual: helpUserManualAction
    property alias libraryCreateCrate: libraryCreateCrateAction
    property alias libraryCreatePlaylist: libraryCreatePlaylistAction
    property alias libraryExport: libraryExportAction
    property alias libraryRescan: libraryRescanAction
    property alias librarySearchCurrentView: librarySearchCurrentViewAction
    property alias librarySearchTracks: librarySearchTracksAction
    required property int numberOfDecks
    property alias optionsEnableKeyboardShortcuts: optionsEnableKeyboardShortcutsAction
    property alias optionsEnableLiveBroadcasting: optionsEnableLiveBroadcastingAction
    property alias optionsEnableVinyl1: optionsEnableVinyl1Action
    property alias optionsEnableVinyl2: optionsEnableVinyl2Action
    property alias optionsEnableVinyl3: optionsEnableVinyl3Action
    property alias optionsEnableVinyl4: optionsEnableVinyl4Action
    property alias optionsPreferences: optionsPreferencesAction
    property alias optionsRecordMix: optionsRecordMixAction
    readonly property string preferencesDefaultShortcut: Qt.platform.os === "osx" ? "Ctrl+," : "Ctrl+P"
    property int shortcutRevision: 0
    property alias viewFullScreen: viewFullScreenAction
    property alias viewMaximizeLibrary: viewMaximizeLibraryAction
    property alias viewShowAutoDJ: viewShowAutoDJAction
    property alias viewShowCoverArt: viewShowCoverArtAction
    property alias viewShowKeywheel: viewShowKeywheelAction
    property alias viewShowMicrophone: viewShowMicrophoneAction
    property alias viewShowPreviewDeck: viewShowPreviewDeckAction
    property alias viewShowVinylControl: viewShowVinylControlAction

    signal focusLibrarySearchRequested

    function configuredMenuShortcut(command, defaultShortcut, revision) {
        return Mixxx.Application.menuShortcut(command, defaultShortcut);
    }

    Connections {
        function onMenuShortcutsChanged() {
            root.shortcutRevision += 1;
        }

        target: Mixxx.Application
    }
    Action {
        id: fileLoadDeck1Action

        shortcut: root.configuredMenuShortcut("FileMenu_LoadDeck1", "Ctrl+O", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Load Track to Deck &%1").arg(1)

        onTriggered: root.commands.loadTrackToDeck(1)
    }
    Action {
        id: fileLoadDeck2Action

        shortcut: root.configuredMenuShortcut("FileMenu_LoadDeck2", "Ctrl+Shift+O", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Load Track to Deck &%1").arg(2)

        onTriggered: root.commands.loadTrackToDeck(2)
    }
    Action {
        id: fileLoadDeck3Action

        enabled: root.numberOfDecks >= 3
        shortcut: root.configuredMenuShortcut("FileMenu_LoadDeck3", "", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Load Track to Deck &%1").arg(3)

        onTriggered: root.commands.loadTrackToDeck(3)
    }
    Action {
        id: fileLoadDeck4Action

        enabled: root.numberOfDecks >= 4
        shortcut: root.configuredMenuShortcut("FileMenu_LoadDeck4", "", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Load Track to Deck &%1").arg(4)

        onTriggered: root.commands.loadTrackToDeck(4)
    }
    Action {
        id: fileQuitAction

        shortcut: root.configuredMenuShortcut("FileMenu_Quit", "Ctrl+Q", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "&Exit")

        onTriggered: Qt.quit()
    }
    Action {
        id: libraryRescanAction

        enabled: !Mixxx.Library.libraryScanActive
        shortcut: root.configuredMenuShortcut("LibraryMenu_Rescan", "Ctrl+Shift+L", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "&Rescan Library")

        onTriggered: Mixxx.Library.rescanLibrary()
    }
    Action {
        id: libraryExportAction

        enabled: Mixxx.Library.enginePrimeExportAvailable
        text: qsTranslate("WMainMenuBar", "E&xport Library to Engine DJ")

        onTriggered: Mixxx.Library.exportLibrary()
    }
    Action {
        id: librarySearchCurrentViewAction

        shortcut: root.configuredMenuShortcut("LibraryMenu_SearchInCurrentView", "Ctrl+F", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Search in Current View...")

        onTriggered: {
            Mixxx.Library.searchInCurrentView();
            root.focusLibrarySearchRequested();
        }
    }
    Action {
        id: librarySearchTracksAction

        shortcut: root.configuredMenuShortcut("LibraryMenu_SearchInAllTracks", "Ctrl+Shift+F", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Search in Tracks Library...")

        onTriggered: {
            Mixxx.Library.searchInTracksLibrary();
            root.focusLibrarySearchRequested();
        }
    }
    Action {
        id: libraryCreatePlaylistAction

        shortcut: root.configuredMenuShortcut("LibraryMenu_NewPlaylist", "Ctrl+N", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Create &New Playlist")

        onTriggered: Mixxx.Library.createPlaylist()
    }
    Action {
        id: libraryCreateCrateAction

        shortcut: root.configuredMenuShortcut("LibraryMenu_NewCrate", "Ctrl+Shift+N", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Create New &Crate")

        onTriggered: Mixxx.Library.createCrate()
    }
    Action {
        id: viewShowMicrophoneAction

        checkable: true
        checked: showMicrophonesControl.value > 0
        shortcut: root.configuredMenuShortcut("ViewMenu_ShowMicrophone", "Ctrl+2", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Microphone Section")

        onTriggered: showMicrophonesControl.toggle()
    }
    Action {
        id: viewShowVinylControlAction

        checkable: true
        checked: showVinylControl.value > 0
        enabled: Mixxx.Application.vinylControlAvailable
        shortcut: root.configuredMenuShortcut("ViewMenu_ShowVinylControl", "Ctrl+3", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Vinyl Control Section")

        onTriggered: showVinylControl.toggle()
    }
    Action {
        id: viewShowPreviewDeckAction

        checkable: true
        checked: showPreviewDecksControl.value > 0
        shortcut: root.configuredMenuShortcut("ViewMenu_ShowPreviewDeck", "Ctrl+4", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Preview Deck")

        onTriggered: showPreviewDecksControl.toggle()
    }
    Action {
        id: viewShowCoverArtAction

        checkable: true
        checked: showLibraryCoverArtControl.value > 0
        shortcut: root.configuredMenuShortcut("ViewMenu_ShowCoverArt", "Ctrl+6", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Cover Art")

        onTriggered: showLibraryCoverArtControl.toggle()
    }
    Action {
        id: viewShowKeywheelAction

        shortcut: root.configuredMenuShortcut("ViewMenu_ShowKeywheel", "F12", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Keywheel")

        onTriggered: root.commands.showKeywheel()
    }
    Action {
        id: viewMaximizeLibraryAction

        checkable: true
        checked: showMaximizedLibraryControl.value > 0
        shortcut: root.configuredMenuShortcut("ViewMenu_MaximizeLibrary", "Space", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Maximize Library")

        onTriggered: showMaximizedLibraryControl.toggle()
    }
    Action {
        id: viewShowAutoDJAction

        shortcut: root.configuredMenuShortcut("ViewMenu_ShowAutoDJ", "Ctrl+9", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Show Auto DJ")

        onTriggered: Mixxx.Library.showAutoDJ()
    }
    Action {
        id: viewFullScreenAction

        checkable: true
        checked: root.applicationWindow.visibility === Window.FullScreen
        shortcut: Qt.platform.os === "osx" ? "Ctrl+Meta+F" : "F11"
        text: qsTranslate("WMainMenuBar", "&Full Screen")

        onTriggered: root.commands.toggleFullScreen()
    }
    Action {
        id: optionsEnableVinyl1Action

        checkable: true
        checked: vinylDeck1Control.value > 0
        enabled: Mixxx.Application.vinylControlAvailable
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableVinyl1", "Ctrl+T", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable Vinyl Control &%1").arg(1)

        onTriggered: vinylDeck1Control.toggle()
    }
    Action {
        id: optionsEnableVinyl2Action

        checkable: true
        checked: vinylDeck2Control.value > 0
        enabled: Mixxx.Application.vinylControlAvailable
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableVinyl2", "Ctrl+Y", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable Vinyl Control &%1").arg(2)

        onTriggered: vinylDeck2Control.toggle()
    }
    Action {
        id: optionsEnableVinyl3Action

        checkable: true
        checked: vinylDeck3Control.value > 0
        enabled: Mixxx.Application.vinylControlAvailable && root.numberOfDecks >= 3
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableVinyl3", "Ctrl+U", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable Vinyl Control &%1").arg(3)

        onTriggered: vinylDeck3Control.toggle()
    }
    Action {
        id: optionsEnableVinyl4Action

        checkable: true
        checked: vinylDeck4Control.value > 0
        enabled: Mixxx.Application.vinylControlAvailable && root.numberOfDecks >= 4
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableVinyl4", "Ctrl+I", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable Vinyl Control &%1").arg(4)

        onTriggered: vinylDeck4Control.toggle()
    }
    Action {
        id: optionsRecordMixAction

        checkable: true
        checked: recordingStatusControl.value > 0
        shortcut: root.configuredMenuShortcut("OptionsMenu_RecordMix", "Ctrl+R", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "&Record Mix")

        onTriggered: recordingToggleControl.trigger()
    }
    Action {
        id: optionsEnableLiveBroadcastingAction

        checkable: true
        checked: broadcastEnabledControl.value > 0
        enabled: Mixxx.Application.liveBroadcastingAvailable
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableLiveBroadcasting", "Ctrl+L", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable Live &Broadcasting")

        onTriggered: broadcastEnabledControl.toggle()
    }
    Action {
        id: optionsEnableKeyboardShortcutsAction

        checkable: true
        checked: Mixxx.Application.keyboardShortcutsEnabled
        shortcut: root.configuredMenuShortcut("OptionsMenu_EnableShortcuts", "Ctrl+`", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Enable &Keyboard Shortcuts")

        onTriggered: Mixxx.Application.keyboardShortcutsEnabled = !Mixxx.Application.keyboardShortcutsEnabled
    }
    Action {
        id: optionsPreferencesAction

        shortcut: root.configuredMenuShortcut("OptionsMenu_Preferences", root.preferencesDefaultShortcut, root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "&Preferences")

        onTriggered: Mixxx.PreferencesDialog.show()
    }
    Action {
        id: developerReloadSkinAction

        enabled: Mixxx.Application.developerMode
        shortcut: root.configuredMenuShortcut("OptionsMenu_ReloadSkin", "Ctrl+Shift+R", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "&Reload Skin")

        onTriggered: Mixxx.Application.reloadSkin()
    }
    Action {
        id: developerToolsAction

        enabled: Mixxx.Application.developerMode
        shortcut: root.configuredMenuShortcut("OptionsMenu_DeveloperTools", "Ctrl+Shift+T", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Developer &Tools")

        onTriggered: root.commands.showDeveloperToolsRequested()
    }
    Action {
        id: developerExperimentStatsAction

        checkable: true
        checked: Mixxx.Application.experimentStatsEnabled
        enabled: Mixxx.Application.developerMode
        shortcut: root.configuredMenuShortcut("OptionsMenu_DeveloperStatsExperiment", "Ctrl+Shift+E", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Stats: &Experiment Bucket")

        onTriggered: Mixxx.Application.setExperimentStatsEnabled(!Mixxx.Application.experimentStatsEnabled)
    }
    Action {
        id: developerBaseStatsAction

        checkable: true
        checked: Mixxx.Application.baseStatsEnabled
        enabled: Mixxx.Application.developerMode
        shortcut: root.configuredMenuShortcut("OptionsMenu_DeveloperStatsBase", "Ctrl+Shift+B", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Stats: &Base Bucket")

        onTriggered: Mixxx.Application.setBaseStatsEnabled(!Mixxx.Application.baseStatsEnabled)
    }
    Action {
        id: developerDebuggerAction

        checkable: true
        checked: Mixxx.Application.debuggerEnabled
        enabled: Mixxx.Application.developerMode
        shortcut: root.configuredMenuShortcut("DeveloperMenu_EnableDebugger", "Ctrl+Shift+D", root.shortcutRevision)
        text: qsTranslate("WMainMenuBar", "Deb&ugger Enabled")

        onTriggered: Mixxx.Application.debuggerEnabled = !Mixxx.Application.debuggerEnabled
    }
    Action {
        id: helpCommunitySupportAction

        text: qsTranslate("WMainMenuBar", "&Community Support") + root.externalLinkSuffix

        onTriggered: Qt.openUrlExternally("https://www.mixxx.org/support/")
    }
    Action {
        id: helpUserManualAction

        text: qsTranslate("WMainMenuBar", "&User Manual") + (Mixxx.Application.userManualExternal ? root.externalLinkSuffix : "")

        onTriggered: Qt.openUrlExternally(Mixxx.Application.userManualUrl)
    }
    Action {
        id: helpKeyboardShortcutsAction

        text: qsTranslate("WMainMenuBar", "&Keyboard Shortcuts") + (Mixxx.Application.keyboardShortcutsExternal ? root.externalLinkSuffix : "")

        onTriggered: Qt.openUrlExternally(Mixxx.Application.keyboardShortcutsUrl)
    }
    Action {
        id: helpSettingsDirectoryAction

        // Qt Quick Controls does not expose native menu roles. Prevent
        // macOS from treating this action as the application Preferences action.
        text: "\u200c" + qsTranslate("WMainMenuBar", "&Settings directory")

        onTriggered: Qt.openUrlExternally(Mixxx.Application.settingsDirectoryUrl)
    }
    Action {
        id: helpTranslateAction

        text: qsTranslate("WMainMenuBar", "&Translate This Application") + root.externalLinkSuffix

        onTriggered: Qt.openUrlExternally("https://explore.transifex.com/mixxx-dj-software/")
    }
    Action {
        id: helpAboutAction

        text: qsTranslate("WMainMenuBar", "&About")

        onTriggered: root.commands.showAbout()
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
