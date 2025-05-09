import "." as Skin
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.6
import "Theme"

Item {
    id: root

    Mixxx.LibrarySourceTree {
        id: librarySources

        component DefaultDelegate: LibraryCell {
            id: cell
            readonly property var caps: capabilities
            Drag.active: dragArea.drag.active

            LibraryTrack {
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
                        });
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
                text: display
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
            // WaveformOverview
            // Mixxx.TrackListColumn {
            //     label: "Preview"
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

            //         LibraryTrack {
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
                label: "Title"
                fillSpan: 3
                columnIdx: Mixxx.TrackListColumn.SQLColumns.Title

                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Artist"
                fillSpan: 2

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Artist
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Album"
                fillSpan: 1

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Year"
                preferredWidth: 80

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Year
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Bpm"
                preferredWidth: 60

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Bpm
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Key"
                preferredWidth: 70

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Key
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "File Type"
                preferredWidth: 70

                columnIdx: Mixxx.TrackListColumn.SQLColumns.FileType
                delegate: DefaultDelegate { }
            },
            Mixxx.TrackListColumn {
                label: "Bitrate"
                preferredWidth: 70

                columnIdx: Mixxx.TrackListColumn.SQLColumns.Bitrate
                delegate: DefaultDelegate { }
            }
        ]
        Mixxx.LibraryAllTrackSource {
            label: "All..."
            columns: librarySources.defaultColumns
        }
        Mixxx.LibraryPlaylistSource {
            id: playlistSource
            label: "Playlist"
            icon: "images/library_playlist.png"

            columns: librarySources.defaultColumns
        }
        Mixxx.LibraryCrateSource {
            id: crateSource
            label: "Crate"
            icon: "images/library_crate.png"

            columns: librarySources.defaultColumns
        }
        Mixxx.LibraryExplorerSource {
            label: "Explorer"
            icon: "images/library_explorer.png"
            columns: [
                Mixxx.TrackListColumn {

                    label: "Preview"
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
                    label: "Filename"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {
                    role: Mixxx.TrackListColumn.Role.Artist

                    label: "Artist"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {
                    role: Mixxx.TrackListColumn.Role.Title

                    label: "Title"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Album"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Track #"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Year"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Genre"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Composer"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Comment"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Duration"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "BPM"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Key"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Type"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Bitrate"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {
                    role: Mixxx.TrackListColumn.Role.Location

                    label: "Location"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Album Artist"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "Grouping"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "File Modified"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "File Created"
                    delegate: DefaultDelegate {}
                },
                Mixxx.TrackListColumn {

                    label: "ReplayGain"
                    delegate: DefaultDelegate {}
                }
            ]
        }
    }

    property var sidebar: librarySources.sidebar()

    SplitView {
        id: librarySplitView
        orientation: Qt.Horizontal
        anchors.fill: parent

        handle: Rectangle {
            id: handleDelegate
            implicitWidth: 8
            implicitHeight: 8
            color: Theme.panelSplitterBackground
            clip: true
            property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
            property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

            ColumnLayout {
                anchors.centerIn: parent
                Repeater {
                    model: 3
                    Rectangle {
                        width: handleSize
                        height: handleSize
                        radius: handleSize
                        color: handleColor
                    }
                }
            }

            containmentMask: Item {
                x: (handleDelegate.width - width) / 2
                width: 8
                height: librarySplitView.height
            }
        }

        SplitView {
            id: sideBarSplitView
            SplitView.minimumWidth: 100
            SplitView.preferredWidth: 415
            SplitView.maximumWidth: 600

            orientation: Qt.Vertical

            handle: Rectangle {
                id: handleDelegate
                implicitWidth: 8
                implicitHeight: 8
                color: Theme.panelSplitterBackground
                clip: true
                property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
                property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

                RowLayout {
                    anchors.centerIn: parent
                    Repeater {
                        model: 3
                        Rectangle {
                            width: handleSize
                            height: handleSize
                            radius: handleSize
                            color: handleColor
                        }
                    }
                }

                containmentMask: Item {
                    x: (handleDelegate.width - width) / 2
                    height: 8
                    width: sideBarSplitView.width
                }
            }
            LibraryBrowser {
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 500
                SplitView.fillHeight: true

                model: root.sidebar
            }

            Skin.PreviewDeck {
                SplitView.minimumHeight: 100
                SplitView.preferredHeight: 100
                SplitView.maximumHeight: 200
            }
        }
        LibraryTrackList {
            SplitView.fillHeight: true

            model: root.sidebar.tracklist
        }
    }
}
