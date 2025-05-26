import QtQuick
import Mixxx 1.0 as Mixxx
import "../Theme"

Mixxx.LibrarySourceTree {
    id: root

    property alias playlist: playlistSource
    property alias crate: crateSource

    defaultColumns: [
        Mixxx.TrackListColumn {
            autoHideWidth: 750
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
            columnType: Mixxx.TrackListColumn.ColumnType.AlbumArt
            preferredWidth: 100
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 850
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Title
            columnType: Mixxx.TrackListColumn.ColumnType.Overview
            fillSpan: 3
            label: qsTr("Preview")
            preferredWidth: 200
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Title
            fillSpan: 3
            label: qsTr("Title")
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Artist
            fillSpan: 2
            label: qsTr("Artist")
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 690
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
            fillSpan: 1
            label: qsTr("Album")
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 750
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Year
            label: qsTr("Year")
            preferredWidth: 80
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bpm
            label: qsTr("BPM")
            preferredWidth: 60
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Key
            label: qsTr("Key")
            preferredWidth: 70
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 900
            columnIdx: Mixxx.TrackListColumn.SQLColumns.FileType
            label: qsTr("File Type")
            preferredWidth: 70
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 1200
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bitrate
            label: qsTr("Bitrate")
            preferredWidth: 70
        }
    ]

    Mixxx.LibraryAllTrackSource {
        columns: root.defaultColumns
        label: qsTr("All...")
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
                autoHideWidth: 850
                columnType: Mixxx.TrackListColumn.ColumnType.Overview
                fillSpan: 3
                label: qsTr("Preview")
                preferredWidth: 200
            },
            Mixxx.TrackListColumn {
                label: qsTr("Filename")
                fillSpan: 4
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Artist
                label: qsTr("Artist")
                fillSpan: 2
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Title
                label: qsTr("Title")
                fillSpan: 2
            },
            Mixxx.TrackListColumn {
                label: qsTr("Album")
                fillSpan: 1
            },
            Mixxx.TrackListColumn {
                label: qsTr("Track #")
                display: Mixxx.TrackListColumn.Display.Hide
                preferredWidth: 60
            },
            Mixxx.TrackListColumn {
                autoHideWidth: 750
                label: qsTr("Year")
                preferredWidth: 80
            },
            Mixxx.TrackListColumn {
                autoHideWidth: 750
                label: qsTr("Genre")
                preferredWidth: 80
            },
            Mixxx.TrackListColumn {
                label: qsTr("Composer")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("Comment")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("Duration")
            },
            Mixxx.TrackListColumn {
                label: qsTr("BPM")
                preferredWidth: 60
            },
            Mixxx.TrackListColumn {
                label: qsTr("Key")
                preferredWidth: 70
            },
            Mixxx.TrackListColumn {
                autoHideWidth: 900
                label: qsTr("File Type")
                preferredWidth: 70
            },
            Mixxx.TrackListColumn {
                autoHideWidth: 1200
                label: qsTr("Bitrate")
                preferredWidth: 70
            },
            Mixxx.TrackListColumn {
                role: Mixxx.TrackListColumn.Role.Location
                label: qsTr("Location")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("Album Artist")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("Grouping")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("File Modified")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("File Created")
                display: Mixxx.TrackListColumn.Display.Hide
            },
            Mixxx.TrackListColumn {
                label: qsTr("ReplayGain")
                display: Mixxx.TrackListColumn.Display.Hide
            }
        ]
    }
}
