import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls 2.15
import "Theme"

MouseArea {
    id: dragArea

    required property var capabilities
    required property var playlists
    required property var crates

    readonly property var library: Mixxx.Library

    drag.target: value
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    onClicked: (mouse) => {
        if (mouse.button === Qt.RightButton)
            contextMenu.popup()
    }
    onPressAndHold: (mouse) => {
        if (mouse.source === Qt.MouseEventNotSynthesized)
            contextMenu.popup()
    }

    function hasCapabilities(caps) {
        return (dragArea.capabilities & caps) == caps;
    }

    Menu {
        id: contextMenu
        title: qsTr("File")

        Menu {
            title: qsTr("Load to")
            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToDeck) ||
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToSampler) ||
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToPreviewDeck)
            }

            Menu {
                id: loadToDeckMenu
                title: qsTr("Deck")
                enabled: hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToDeck)
                Instantiator {
                    model: 4
                    delegate: MenuItem {
                        text: qsTr("Deck %1").arg(modelData+1)
                        onTriggered: Mixxx.PlayerManager.getPlayer(`[Channel${modelData+1}]`).loadTrack(track)
                    }

                    onObjectAdded: (index, object) => loadToDeckMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => loadToDeckMenu.removeItem(object)
                }
            }

            Menu {
                title: qsTr("Sampler")
                enabled: hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToSampler)
            }

            // Instantiator {
            //     id: recentFilesInstantiator
            //     model: settings.recentFiles
            //     delegate: MenuItem {
            //         text: settings.displayableFilePath(modelData)
            //         onTriggered: loadFile(modelData)
            //     }

            //     onObjectAdded: (index, object) => recentFilesMenu.insertItem(index, object)
            //     onObjectRemoved: (index, object) => recentFilesMenu.removeItem(object)
            // }
        }

        Menu {
            id: addToPlaylistMenu
            title: qsTr("Add to playlists")
            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.AddToTrackSet)
            }

            Instantiator {
                model: playlists.list()
                delegate: MenuItem {
                    text: modelData.name
                    onTriggered: modelData.addTrack(track)
                    enabled: !modelData.locked
                }

                onObjectAdded: (index, object) => addToPlaylistMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => addToPlaylistMenu.removeItem(object)
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("Create New Playlist")
                onTriggered: playlists.createPlaylist()
            }
        }

        Menu {
            id: addToCrateMenu
            title: qsTr("Crates")
            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.AddToTrackSet)
            }

            Instantiator {
                model: crates.list([track])
                delegate: MenuItem {
                    text: modelData.name
                    checked: modelData.trackCount() == 1
                    checkable: true
                    onToggled: {
                        if (checked) {
                            modelData.addTrack(track)
                        } else {
                            modelData.removeTrack(track)
                        }
                    }
                    enabled: !modelData.locked
                }

                onObjectAdded: (index, object) => addToCrateMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => addToCrateMenu.removeItem(object)
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("Create New Playlist")
                onTriggered: playlists.createPlaylist()
            }
        }

        Menu {
            id: analyzeMenu
            title: qsTr("Analyze")
            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.EditMetadata)||
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.Analyze)
            }
            MenuItem {
                text: qsTr("Analyze")
                onTriggered: {
                    library.analyze(track)
                }
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze")
                onTriggered: playlists.createPlaylist()
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze (constant BPM)")
                onTriggered: playlists.createPlaylist()
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze (variable BPM)")
                onTriggered: playlists.createPlaylist()
            }
        }
    }
}
