import QtQuick
import Mixxx 1.0 as Mixxx
import "." as LibraryComponent
import "../Theme"

Mixxx.LibrarySourceTree {
    id: root

    component DefaultDelegate: LibraryComponent.Cell {
        id: cell
        readonly property var caps: capabilities
        property bool customRender: false
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
            onPressed: {
                tableView.selectionModel.selectRow(row)
            }
            onDoubleClicked: {
                tableView.loadSelectedTrackIntoNextAvailableDeck(false);
            }
        }

        Component.onCompleted: updateDragImage()

        function updateDragImage() {
            cell.dragImage.grabToImage((result) => {
                    cell.Drag.imageSource = result.url;
                }, Qt.size(cell.dragImage.width, cell.dragImage.height))
        }

        Connections {
            target: parent
            function onTrackChanged() {
                cell.updateDragImage()
            }
        }

        Text {
            id: value
            visible: !customRender
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
        Mixxx.TrackListColumn {
            label: qsTr("Preview")
            fillSpan: 3
            preferredWidth: 200
            columnIdx: Mixxx.TrackListColumn.SQLColumns.Title

            delegate: DefaultDelegate {
                anchors.fill: parent

                readonly property var trackProxy: track
                customRender: true

                onTrackProxyChanged: {
                    if (trackProxy && !trackProxy.hasWaveform) {
                        Mixxx.Library.analyze(trackProxy)
                    }
                }

                Mixxx.WaveformOverview {
                    anchors.fill: parent
                    channels: Mixxx.WaveformOverview.Channels.LeftChannel
                    renderer: Mixxx.WaveformOverview.Renderer.Filtered
                    colorHigh: Theme.white
                    colorMid: Theme.blue
                    colorLow: Theme.green
                    track: trackProxy
                }
                Rectangle {
                    id: border
                    color: Theme.darkGray2
                    width: 1
                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        right: parent.right
                    }
                }
            }

        },
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
}
