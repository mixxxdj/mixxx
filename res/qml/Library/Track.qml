import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls 2.15
import "../Theme"
import ".." as Skin

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
                            playlists.get(text).addTrack(track)
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
