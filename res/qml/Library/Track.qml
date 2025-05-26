import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2.15
import "../Theme"
import ".." as Skin

Item {
    id: root

    readonly property var library: Mixxx.Library

    required property var capabilities
    required property var view
    required property var playlists
    required property var crates

    property alias tap: tapHandler

    property var _lazyTrack: null

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
            view.selectionModel.selectRow(row);
            view.loadSelectedTrackIntoNextAvailableDeck(false);
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

            Instantiator {
                model: playlists.list()
                delegate: MenuItem {
                    text: modelData.name
                    onTriggered: modelData.addTrack(root._lazyTrack)
                    enabled: !modelData.locked
                }

                onObjectAdded: (index, object) => addToPlaylistMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => addToPlaylistMenu.removeItem(object)
            }

            MenuSeparator {}

            MenuItem {
                id: createPlaylistItem

                property bool creating: false

                text: qsTr("Create New Playlist")
                contentItem: Item {
                    TextInput {
                        id: playlistNewName
                        visible: createPlaylistItem.creating
                        anchors.fill: parent
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.margins: 7
                        focus: true
                        clip: true
                        color: acceptableInput ? "#000000" : 'red'
                        horizontalAlignment: TextInput.AlignLeft
                        onAccepted: {
                            if (!text) {
                                return
                            }
                            let result = playlists.create(text)
                            if (result != Mixxx.LibraryPlaylistSource.PlaylistCreateResult.Ok) {
                                // TODO UX feedback
                                console.warn("Create New Playlist", text, result)
                                return
                            }
                            playlists.get(text).addTrack(root._lazyTrack)
                            text = ""
                            createPlaylistItem.creating = false
                        }
                    }
                    Text {
                        visible: !createPlaylistItem.creating
                        leftPadding: createPlaylistItem.indicator.width
                        rightPadding: createPlaylistItem.arrow.width
                        text: createPlaylistItem.text
                        font: createPlaylistItem.font
                        opacity: enabled ? 1.0 : 0.3
                        color: "#000000"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    TapHandler {
                        onTapped: {
                            createPlaylistItem.creating = true
                            playlistNewName.forceActiveFocus();
                        }
                    }
                }
            }
        }
        Menu {
            id: addToCrateMenu

            enabled: {
                hasCapabilities(Mixxx.LibraryTrackListModel.Capability.AddToTrackSet);
            }
            title: qsTr("Crates")

            Instantiator {
                model: root._lazyTrack ? crates.list([root._lazyTrack]) : 0
                delegate: MenuItem {
                    text: modelData.name
                    checked: modelData.trackCount() == 1
                    checkable: true
                    onToggled: {
                        if (checked) {
                            modelData.addTrack(root._lazyTrack)
                        } else {
                            modelData.removeTrack(root._lazyTrack)
                        }
                    }
                    enabled: !modelData.locked
                }

                onObjectAdded: (index, object) => addToCrateMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => addToCrateMenu.removeItem(object)
            }

            MenuSeparator {}

            MenuItem {
                id: createCrateItem

                property bool creating: false

                text: qsTr("Create New Crate")
                contentItem: Item {
                    TextInput {
                        id: crateNewName
                        visible: createCrateItem.creating
                        anchors.fill: parent
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.margins: 7
                        focus: true
                        clip: true
                        color: acceptableInput ? "#000000" : 'red'
                        horizontalAlignment: TextInput.AlignLeft
                        onAccepted: {
                            if (text) {
                                crates.create(text)
                            }
                            text = ""
                            createCrateItem.creating = false
                        }
                    }
                    Text {
                        visible: !createCrateItem.creating
                        leftPadding: createCrateItem.indicator.width
                        rightPadding: createCrateItem.arrow.width
                        text: createCrateItem.text
                        font: createCrateItem.font
                        opacity: enabled ? 1.0 : 0.3
                        color: "#000000"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    TapHandler {
                        onTapped: {
                            createPlaylistItem.creating = true
                            playlistNewName.forceActiveFocus();
                        }
                    }
                }
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
