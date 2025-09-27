import QtQuick
import Mixxx 1.0 as Mixxx
import "." as LibraryComponent
import "../Theme"

Mixxx.LibrarySourceTree {
    id: root

    component DefaultDelegate: LibraryComponent.Cell {
        id: cell
        readonly property var caps: capabilities
        // FIXME: https://bugreports.qt.io/browse/QTBUG-111789
        Binding on Drag.active {
            value: dragArea.drag.active
            // This delays the update until the even queue is cleared
            // preventing any potential oscillations causing a loop
            delayed: true
        }

        LibraryComponent.Track {
            id: dragArea
            anchors.fill: parent
            capabilities: cell.caps
            playlists: playlistSource
            crates: crateSource

            onPressed: {
                if (pressedButtons == Qt.LeftButton) {
                    tableView.selectionModel.selectRow(row);
                    parent.dragImage.grabToImage((result) => {
                            parent.Drag.imageSource = result.url;
                        }, Qt.size(parent.dragImage.width, parent.dragImage.height));
                }
            }
            onDoubleClicked: {
                tableView.selectionModel.selectRow(row);
                tableView.loadSelectedTrackIntoNextAvailableDeck(false);
            }
        }

        Text {
            id: value
            anchors.fill: parent
            anchors.leftMargin: 15
            font.pixelSize: 14
            text: display ?? ""
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: Theme.textColor
        }
    }

    defaultColumns: [
        Mixxx.TrackListColumn {
            preferredWidth: 110

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album

            delegate: Rectangle {
                color: decoration
                implicitHeight: 30

                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: cover_art
                    clip: true
                    asynchronous: true
                }
            }
        },
        // FIXME: WaveformOverview is currently disabled due to performance limitation. Like for the legacy UI, a cache likely needs to be implemented to help
        // Mixxx.TrackListColumn {
        //     label: qsTr("Preview")
        //     fillSpan: 3
        //     preferredWidth: 300
        //     columnIdx: Mixxx.TrackListColumn.SQLColumns.Title

        //     delegate: LibraryCell {
        //         // implicitHeight: 30
        //         anchors.fill: parent

        //         readonly property var trackProxy: track

        //         Drag.active: dragArea.drag.active
        //         Drag.dragType: Drag.Automatic
        //         Drag.supportedActions: Qt.CopyAction
        //         Drag.mimeData: {
        //             "text/uri-list": file_url,
        //             "text/plain": file_url
        //         }

        //         LibraryComponent.Track {
        //             id: dragArea
        //             anchors.fill: parent
        //             capabilities: parent.capabilities

        //             onPressed: {
        //                 if (pressedButtons == Qt.LeftButton) {
        //                     tableView.selectionModel.selectRow(row);
        //                     parent.dragImage.grabToImage((result) => {
        //                             parent.Drag.imageSource = result.url;
        //                     });
        //                 } else {
        //                 }
        //             }
        //             onDoubleClicked: {
        //                 tableView.selectionModel.selectRow(row);
        //                 tableView.loadSelectedTrackIntoNextAvailableDeck(false);
        //             }
        //         }

        //         Mixxx.WaveformOverview {
        //             anchors.fill: parent
        //             channels: Mixxx.WaveformOverview.Channels.LeftChannel
        //             renderer: Mixxx.WaveformOverview.Renderer.Filtered
        //             colorHigh: Theme.white
        //             colorMid: Theme.blue
        //             colorLow: Theme.green
        //             track: trackProxy
        //         }
        //         Rectangle {
        //             id: border
        //             color: Theme.darkGray2
        //             width: 1
        //             anchors {
        //                 top: parent.top
        //                 bottom: parent.bottom
        //                 right: parent.right
        //             }
        //         }
        //     }

        // },
        Mixxx.TrackListColumn {
            label: qsTr("Title")
            fillSpan: 3
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Title

            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Artist")
            fillSpan: 2

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Artist
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Album")
            fillSpan: 1

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Year")
            preferredWidth: 80

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Year
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Bpm")
            preferredWidth: 60

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bpm
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Key")
            preferredWidth: 70

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Key
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("File Type")
            preferredWidth: 70

            columnIdx: Mixxx.TrackListColumn.SQLColumns.FileType
            delegate: DefaultDelegate { }
        },
        Mixxx.TrackListColumn {
            label: qsTr("Bitrate")
            preferredWidth: 70

            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bitrate
            delegate: DefaultDelegate { }
        }
    ]
    Mixxx.LibraryAllTrackSource {
        label: qsTr("All...")
        columns: root.defaultColumns
    }
    Mixxx.LibraryPlaylistSource {
        id: playlistSource
        label: qsTr("Playlist")
        itemName: qsTr("playlist")
        capabilities: Mixxx.LibrarySource.Capability.Create | Mixxx.LibrarySource.Capability.AddTrack
        onRequestCreate: (name) => {
            // TODO create a new item with given name
            print("onRequestCreate", name)
        }
        onRequestAddTrack: (item, track) => {
            // TODO add track to current item
            print("onRequestAddTrack", item, track)
        }
        icon: "../images/library_playlist.png"

        columns: root.defaultColumns
    }
    Mixxx.LibraryCrateSource {
        id: crateSource
        label: qsTr("Crate")
        itemName: qsTr("crate")
        capabilities: Mixxx.LibrarySource.Capability.Create | Mixxx.LibrarySource.Capability.AddTrack
        onRequestCreate: (name) => {
            // TODO create a new item with given name
            print("onRequestCreate", name)
        }
        onRequestAddTrack: (item, track) => {
            // TODO add track to current item
            print("onRequestAddTrack", item, track)
        }
        icon: "../images/library_crates.png"

        columns: root.defaultColumns
    }
    Mixxx.LibraryExplorerSource {
        label: qsTr("Explorer")
        icon: "../images/library_explorer.png"
        columns: [
            Mixxx.TrackListColumn {

                label: qsTr("Preview")
                delegate: Rectangle {
                    color: decoration
                    implicitHeight: 30

                    Image {
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        source: cover_art
                        clip: true
                        asynchronous: true
                    }
                }
            },
            Mixxx.TrackListColumn {
                label: qsTr("Filename")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Artist

                label: qsTr("Artist")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Title

                label: qsTr("Title")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Album")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Track #")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Year")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Genre")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Composer")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Comment")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Duration")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("BPM")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Key")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Type")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Bitrate")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Location

                label: qsTr("Location")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Album Artist")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("Grouping")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("File Modified")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("File Created")
                delegate: DefaultDelegate {}
            },
            Mixxx.TrackListColumn {

                label: qsTr("ReplayGain")
                delegate: DefaultDelegate {}
            }
        ]
    }
}
