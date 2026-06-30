import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2.15
import "../Theme"

Item {
    id: root

    required property var capabilities
    required property var view
    property var _lazyTrack: null
    // property alias drag: dragHandler
    readonly property var library: Mixxx.Library
    property alias tap: tapHandler

    function hasCapabilities(caps) {
        return (root.capabilities & caps) == caps;
    }


    TapHandler {
        id: tapHandler

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onTapped: (eventPoint, button) => {
            view.selectionModel.selectRow(row);
            if (button === Qt.RightButton) {
                _lazyTrack = view.model?.getTrackByRow(row) ?? null;
                contextMenu.popup();
            }
        }
        onDoubleTapped: (eventPoint, button) => {
            if (button === Qt.LeftButton) {
                view.selectionModel.selectRow(row);
                view.loadSelectedTrackIntoNextAvailableDeck(false);
            }
        }
        onLongPressed: (eventPoint, button) => {
            view.selectionModel.selectRow(row);
            _lazyTrack = view.model?.getTrackByRow(row) ?? null;
            contextMenu.popup();
        }
    }
    Menu {
        id: contextMenu

        title: qsTr("File")

        Menu {
            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToDeck) || hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToSampler) || hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToPreviewDeck);
            }
            title: qsTr("Load to")

            Menu {
                id: loadToDeckMenu

                enabled: hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToDeck)
                title: qsTr("Deck")

                Instantiator {
                    model: 4

                    delegate: MenuItem {
                        text: qsTr("Deck %1").arg(modelData + 1)

                        onTriggered: {
                            if (_lazyTrack) Mixxx.PlayerManager.getPlayer(`[Channel${modelData + 1}]`).loadTrack(_lazyTrack);
                        }
                    }

                    onObjectAdded: (index, object) => loadToDeckMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => loadToDeckMenu.removeItem(object)
                }
            }
            Menu {
                enabled: hasCapabilities(Mixxx.LibraryTrackListModel.Capability.LoadToSampler)
                title: qsTr("Sampler")
            }
        }
        Menu {
            id: addToPlaylistMenu

            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.AddToTrackSet);
            }
            title: qsTr("Add to playlists")

            MenuSeparator {
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Create New Playlist")
            }
        }
        Menu {
            id: addToCrateMenu

            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.AddToTrackSet);
            }
            title: qsTr("Crates")

            MenuSeparator {
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Create New Crate")
            }
        }
        Menu {
            id: analyzeMenu

            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.EditMetadata) || hasCapabilities(Mixxx.LibraryTrackListModel.Capability.Analyze);
            }
            title: qsTr("Analyze")

            MenuItem {
                text: qsTr("Analyze")

                onTriggered: {
                    if (_lazyTrack) library.analyze(_lazyTrack);
                }
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze")
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze (constant BPM)")
            }
            MenuItem {
                enabled: false // TODO implement
                text: qsTr("Reanalyze (variable BPM)")
            }
        }
    }
}
