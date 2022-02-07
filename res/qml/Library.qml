import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQuick
import QtQuick.Controls 2.15
import "Theme"

Item {
    Rectangle {
        color: Theme.deckBackgroundColor
        anchors.fill: parent

        LibraryControl {
            id: libraryControl

            onMoveVertical: (offset) => {
                tableView.selectionModel.moveSelectionVertical(offset);
            }
            onLoadSelectedTrack: (group, play) => {
                tableView.loadSelectedTrack(group, play);
            }
            onLoadSelectedTrackIntoNextAvailableDeck: (play) => {
                tableView.loadSelectedTrackIntoNextAvailableDeck(play);
            }
            onFocusWidgetChanged: {
                switch (focusWidget) {
                    case FocusedWidgetControl.WidgetKind.LibraryView:
                        tableView.forceActiveFocus();
                        break;
                }
            }
        }

        HorizontalHeaderView {
            id: horizontalHeader

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 5
            syncView: tableView
            model: ["Color", "Cover", "Artist", "Album", "Year", "Bpm", "Key", "Filetype", "Bitrate"]

            delegate: Item {
                id: headerDlgt

                required property int column
                required property string modelData

                implicitHeight: columnName.contentHeight + 5
                implicitWidth: columnName.contentWidth + 5

                BorderImage {
                    anchors.fill: parent
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    source: Theme.imgPopupBackground

                    border {
                        top: 10
                        left: 20
                        right: 20
                        bottom: 10
                    }
                }

                Text {
                    id: columnName

                    text: headerDlgt.modelData
                    anchors.fill: parent
                    anchors.margins: 5
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.fontFamily
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    font.pixelSize: Theme.buttonFontPixelSize
                    color: Theme.buttonNormalColor
                }

                Text {
                    id: sortIndicator

                    anchors.fill: parent
                    anchors.margins: 5
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.fontFamily
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    font.pixelSize: Theme.buttonFontPixelSize
                    color: Theme.buttonNormalColor
                }
            }
        }

        TableView {
            id: tableView

            function loadSelectedTrackIntoNextAvailableDeck(play) {
                const urls = this.selectionModel.selectedTrackUrls();
                if (urls.length == 0)
                    return ;

                Mixxx.PlayerManager.loadLocationUrlIntoNextAvailableDeck(urls[0], play);
            }

            function loadSelectedTrack(group, play) {
                const urls = this.selectionModel.selectedTrackUrls();
                if (urls.length == 0)
                    return ;

                player.loadTrackFromLocationUrl(urls[0], play);
            }

            anchors.top: horizontalHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 5
            clip: true
            focus: true
            reuseItems: false
            Keys.onUpPressed: this.selectionModel.moveSelectionVertical(-1)
            Keys.onDownPressed: this.selectionModel.moveSelectionVertical(1)
            Keys.onEnterPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
            Keys.onReturnPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
            columnWidthProvider: function(column) {
                switch (column) {
                    case 0:
                        return 30;
                    case 1:
                        return 50;
                    case 2:
                        case 3:
                        case 4:
                        return 300;
                    default:
                        return 100;
                }
            }

            model: Mixxx.TableFromListModel {
                sourceModel: Mixxx.Library.model

                Mixxx.TableFromListModelColumn {
                    decoration: "color"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "coverArtUrl"
                    decoration: "coverArtColor"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "artist"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "album"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "year"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "bpm"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "key"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "fileType"
                    edit: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "bitrate"
                    edit: "fileUrl"
                }
            }

            selectionModel: ItemSelectionModel {
                function selectRow(row) {
                    const rowCount = this.model.rowCount();
                    if (rowCount == 0) {
                        this.clear();
                        return ;
                    }
                    const newRow = Mixxx.MathUtils.positiveModulo(row, rowCount);
                    this.select(this.model.index(newRow, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select | ItemSelectionModel.Clear | ItemSelectionModel.Current);
                }

                function moveSelectionVertical(value) {
                    if (value == 0)
                        return ;

                    const selected = this.selectedIndexes;
                    const oldRow = (selected.length == 0) ? 0 : selected[0].row;
                    this.selectRow(oldRow + value);
                }

                function selectedTrackUrls() {
                    return this.selectedIndexes.map((index) => {
                            return this.model.sourceModel.get(index.row).fileUrl;
                    });
                }

                model: tableView.model
            }

            delegate: DelegateChooser {
                DelegateChoice {
                    column: 0

                    Rectangle {
                        id: trackColorDelegate

                        required property bool selected
                        required property color decoration

                        implicitWidth: 30
                        implicitHeight: 30
                        color: trackColorDelegate.decoration
                    }
                }

                DelegateChoice {
                    column: 1

                    Rectangle {
                        id: coverArtDelegate

                        required property color decoration
                        required property url display

                        implicitWidth: 60
                        implicitHeight: 30
                        color: coverArtDelegate.decoration

                        Image {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: coverArtDelegate.display
                            clip: true
                            asynchronous: true
                        }
                    }
                }

                DelegateChoice {
                    Item {
                        id: itemDelegate

                        required property int row
                        required property bool selected
                        required property string display

                        implicitWidth: 300
                        implicitHeight: 30

                        Text {
                            anchors.fill: parent
                            text: itemDelegate.display
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: itemDelegate.selected ? Theme.blue : Theme.white
                        }

                        Image {
                            id: dragItem

                            Drag.active: dragArea.drag.active
                            Drag.dragType: Drag.Automatic
                            Drag.supportedActions: Qt.CopyAction
                            Drag.mimeData: {
                                "text/uri-list": edit,
                                "text/plain": edit
                            }
                            anchors.fill: parent
                        }

                        MouseArea {
                            id: dragArea

                            anchors.fill: parent
                            drag.target: dragItem
                            onPressed: {
                                tableView.selectionModel.selectRow(itemDelegate.row);
                                parent.grabToImage((result) => {
                                        dragItem.Drag.imageSource = result.url;
                                });
                            }
                            onDoubleClicked: {
                                tableView.selectionModel.selectRow(itemDelegate.row);
                                tableView.loadSelectedTrackIntoNextAvailableDeck(false);
                            }
                        }
                    }
                }
            }
        }
    }
}
