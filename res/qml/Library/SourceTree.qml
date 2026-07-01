import QtQuick
import Mixxx 1.0 as Mixxx
import "." as LibraryComponent
import "../Theme"

Mixxx.LibrarySourceTree {
    id: root

    defaultColumns: [
        Mixxx.TrackListColumn {
            autoHideWidth: 750
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
            preferredWidth: 100

            delegate: Rectangle {
                color: decoration
                implicitHeight: 30

                Image {
                    anchors.fill: parent
                    asynchronous: true
                    clip: true
                    fillMode: Image.PreserveAspectCrop
                    source: cover_art
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
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Title
            fillSpan: 3
            label: qsTr("Title")

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Artist
            fillSpan: 2
            label: qsTr("Artist")

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 690
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Album
            fillSpan: 1
            label: qsTr("Album")

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 750
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Year
            label: qsTr("Year")
            preferredWidth: 80

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bpm
            label: qsTr("Bpm")
            preferredWidth: 60

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Key
            label: qsTr("Key")
            preferredWidth: 70

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 900
            columnIdx: Mixxx.TrackListColumn.SQLColumns.FileType
            label: qsTr("File Type")
            preferredWidth: 70

            delegate: DefaultDelegate {
            }
        },
        Mixxx.TrackListColumn {
            autoHideWidth: 1200
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Bitrate
            label: qsTr("Bitrate")
            preferredWidth: 70

            delegate: DefaultDelegate {
            }
        }
    ]

    Mixxx.LibraryAllTrackSource {
        columns: root.defaultColumns
        label: qsTr("All...")
    }

    component DefaultDelegate: LibraryComponent.Cell {
        id: cell

        readonly property var caps: capabilities

        // FIXME: https://bugreports.qt.io/browse/QTBUG-111789
        Binding on Drag.active {
            // This delays the update until the even queue is cleared
            // preventing any potential oscillations causing a loop
            delayed: true
            value: dragArea.drag.active
        }

        LibraryComponent.Track {
            id: dragArea

            anchors.fill: parent
            capabilities: cell.caps

            drag.onGrabChanged: (transition, eventPoint) => {
                if (transition != PointerDevice.GrabPassive && transition != PointerDevice.GrabExclusive) {
                    return;
                }
                parent.dragImage.grabToImage(result => {
                    parent.Drag.imageSource = result.url;
                }, Qt.size(parent.dragImage.width, parent.dragImage.height));
            }
            tap.onDoubleTapped: {
                tableView.selectionModel.selectRow(row);
                tableView.loadSelectedTrackIntoNextAvailableDeck(false);
            }
            tap.onTapped: (eventPoint, button) => {
                if (button == Qt.LeftButton) {
                    tableView.selectionModel.selectRow(row);
                }
            }
        }
        Text {
            id: value

            anchors.fill: parent
            anchors.leftMargin: 15
            color: Theme.textColor
            elide: Text.ElideRight
            font.pixelSize: 14
            text: display ?? ""
            verticalAlignment: Text.AlignVCenter
        }
    }
}
