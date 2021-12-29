import Mixxx 1.0 as Mixxx
import QtQuick
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

            anchors.fill: parent
            anchors.margins: 5
            clip: true
            focus: true
            Keys.onUpPressed: this.selectionModel.moveSelectionVertical(-1)
            Keys.onDownPressed: this.selectionModel.moveSelectionVertical(1)
            Keys.onEnterPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
            Keys.onReturnPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)

            model: Mixxx.TableFromListModel {
                sourceModel: Mixxx.Library.model

                Mixxx.TableFromListModelColumn {
                    display: "title"
                    decoration: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "artist"
                    decoration: "fileUrl"
                }

                Mixxx.TableFromListModelColumn {
                    display: "album"
                    decoration: "fileUrl"
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

            delegate: Item {
                id: itemDelegate

                required property int row
                required property int column
                required property bool selected
                required property string decoration
                required property string display

                implicitWidth: 300
                implicitHeight: 30

                Text {
                    anchors.fill: parent
                    text: display
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
                        "text/uri-list": itemDelegate.decoration,
                        "text/plain": itemDelegate.decoration
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
