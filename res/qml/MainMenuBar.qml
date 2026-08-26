pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls

MenuBar {
    id: root

    required property ApplicationMenuActions actions
    property Menu developerMenu: null

    Component.onCompleted: {
        if (Mixxx.Application.developerMode) {
            developerMenu = developerMenuComponent.createObject(root);
            root.insertMenu(root.count - 1, developerMenu);
        }
    }

    Menu {
        title: qsTranslate("WMainMenuBar", "&File")

        MenuItem {
            action: root.actions.fileLoadDeck1
        }
        MenuItem {
            action: root.actions.fileLoadDeck2
        }
        MenuItem {
            action: root.actions.fileLoadDeck3
            enabled: root.actions.fileLoadDeck3.enabled
        }
        MenuItem {
            action: root.actions.fileLoadDeck4
            enabled: root.actions.fileLoadDeck4.enabled
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.fileQuit
        }
    }
    Menu {
        title: qsTranslate("WMainMenuBar", "&Library")

        MenuItem {
            action: root.actions.libraryRescan
        }
        MenuItem {
            action: root.actions.libraryExport
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.librarySearchCurrentView
        }
        MenuItem {
            action: root.actions.librarySearchTracks
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.libraryCreatePlaylist
        }
        MenuItem {
            action: root.actions.libraryCreateCrate
        }
    }
    Menu {
        title: qsTranslate("WMainMenuBar", "&View") + "\u200c"

        MenuItem {
            action: root.actions.viewShowMicrophone
        }
        MenuItem {
            action: root.actions.viewShowVinylControl
        }
        MenuItem {
            action: root.actions.viewShowPreviewDeck
        }
        MenuItem {
            action: root.actions.viewShowCoverArt
        }
        MenuItem {
            action: root.actions.viewShowKeywheel
        }
        MenuItem {
            action: root.actions.viewMaximizeLibrary
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.viewShowAutoDJ
        }
        MenuItem {
            action: root.actions.viewFullScreen
        }
    }
    Menu {
        title: qsTranslate("WMainMenuBar", "&Options")

        Menu {
            enabled: Mixxx.Application.vinylControlAvailable
            title: qsTranslate("WMainMenuBar", "&Vinyl Control")

            MenuItem {
                action: root.actions.optionsEnableVinyl1
                enabled: root.actions.optionsEnableVinyl1.enabled
            }
            MenuItem {
                action: root.actions.optionsEnableVinyl2
                enabled: root.actions.optionsEnableVinyl2.enabled
            }
            MenuItem {
                action: root.actions.optionsEnableVinyl3
                enabled: root.actions.optionsEnableVinyl3.enabled
            }
            MenuItem {
                action: root.actions.optionsEnableVinyl4
                enabled: root.actions.optionsEnableVinyl4.enabled
            }
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.optionsRecordMix
        }
        MenuItem {
            action: root.actions.optionsEnableLiveBroadcasting
        }
        MenuItem {
            action: root.actions.optionsEnableKeyboardShortcuts
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.optionsPreferences
        }
    }
    Component {
        id: developerMenuComponent

        Menu {
            title: qsTranslate("WMainMenuBar", "&Developer")

            MenuItem {
                action: root.actions.developerReloadSkin
            }
            MenuItem {
                action: root.actions.developerTools
            }
            MenuItem {
                action: root.actions.developerExperimentStats
            }
            MenuItem {
                action: root.actions.developerBaseStats
            }
            MenuItem {
                action: root.actions.developerDebugger
            }
        }
    }
    Menu {
        title: qsTranslate("WMainMenuBar", "&Help")

        MenuItem {
            action: root.actions.helpCommunitySupport
        }
        MenuItem {
            action: root.actions.helpUserManual
        }
        MenuItem {
            action: root.actions.helpKeyboardShortcuts
        }
        MenuItem {
            action: root.actions.helpSettingsDirectory
        }
        MenuItem {
            action: root.actions.helpTranslate
        }
        MenuSeparator {
        }
        MenuItem {
            action: root.actions.helpAbout
        }
    }
}
