import ".." as Skin
import "." as LibraryComponent
import Mixxx 1.0 as Mixxx
import Qt.labs.qmlmodels
import QtQml
import QtQuick
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls 2.15
import "../Theme"

Rectangle {
    id: root

    color: Theme.darkGray

    required property var model

    LibraryComponent.Control {
        id: libraryControl

        onMoveVertical: (offset) => {
            view.selectionModel.moveSelectionVertical(offset);
        }
        onLoadSelectedTrack: (group, play) => {
            view.loadSelectedTrack(group, play);
        }
        onLoadSelectedTrackIntoNextAvailableDeck: (play) => {
            view.loadSelectedTrackIntoNextAvailableDeck(play);
        }
        onFocusWidgetChanged: {
            switch (focusWidget) {
                case Skin.FocusedWidgetControl.WidgetKind.LibraryView:
                    view.forceActiveFocus();
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
        syncView: view

        property int sortingColumn: -1
        property var sortingOrder: Qt.Descending

        delegate: Item {
            id: column
            required property string display
            required property int index

            implicitHeight: columnName.contentHeight + 5
            implicitWidth: columnName.contentWidth + 5

            MouseArea {
                id: columnMouseHandler
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    if (horizontalHeader.sortingColumn == index) {
                        horizontalHeader.sortingOrder = horizontalHeader.sortingOrder == Qt.DescendingOrder ? Qt.AscendingOrder : Qt.DescendingOrder
                    } else {
                        horizontalHeader.sortingColumn = index
                        horizontalHeader.sortingOrder = Qt.AscendingOrder
                    }
                    view.model.sort(horizontalHeader.sortingColumn, horizontalHeader.sortingOrder);
                }
            }

            Text {
                id: columnName

                text: display
                anchors.fill: parent
                anchors.leftMargin: 15
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.family: Theme.fontFamily
                font.capitalization: Font.Capitalize
                font.pixelSize: 12
                font.weight: Font.Medium
                color: Theme.textColor
            }

            Item {
                anchors {
                    left: parent.left
                    leftMargin: 5
                    top: parent.top
                    bottom: parent.bottom
                }
                Label {
                    id: sortIndicator

                    visible: horizontalHeader.sortingColumn == index

                    text: "▶"
                    rotation: horizontalHeader.sortingOrder == Qt.AscendingOrder ? 90 : -90

                    anchors.centerIn: parent
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.fontFamily
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    font.pixelSize: Theme.buttonFontPixelSize
                    color: "red"
                }
            }
            Rectangle {
                id: columnResizer
                color: Theme.darkGray2
                width: 1
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
                MouseArea {
                    id: columnResizeHandler

                    property int sizeOffset: 0

                    anchors.fill: parent
                    preventStealing: true
                    drag {
                        target: parent
                        axis: Drag.XAxis
                        threshold: 2
                        onActiveChanged: {
                            if (!drag.active && columnResizeHandler.sizeOffset !== 0) {
                                view.model.columns[index].preferredWidth = column.width
                                columnResizeHandler.sizeOffset = 0
                                view.updateColumnSize()
                                view.forceLayout()
                            }
                        }
                    }
                    cursorShape: Qt.SizeHorCursor
                    onMouseXChanged: {
                        if (drag.active) {
                            column.width += mouseX
                            sizeOffset += mouseX
                        }
                    }
                }
            }
        }
    }

    TableView {
        id: view

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

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
        }

        anchors.top: horizontalHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        clip: true
        reuseItems: true
        Keys.onUpPressed: this.selectionModel.moveSelectionVertical(-1)
        Keys.onDownPressed: this.selectionModel.moveSelectionVertical(1)
        Keys.onEnterPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
        Keys.onReturnPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
        model: root.model
        function updateColumnSize() {
            usedWidth = 0;
            dynamicColumnCount = 0;
            if (model == null) {
                return;
            }
            for (let c = 0; c < model.columns.length; c++) {
                if (model.columns[c].hidden) {
                    continue
                } else if (model.columns[c].preferredWidth > 0) {
                    usedWidth += model.columns[c].preferredWidth;
                } else {
                    dynamicColumnCount += model.columns[c].fillSpan || 1
                }
            }
        }
        Component.onCompleted: this.updateColumnSize()
        onModelChanged: this.updateColumnSize()

        property int usedWidth: 0
        property int dynamicColumnCount: 0

        columnWidthProvider: function(column) {
            const columnDef = view.model.columns[column]
            if (columnDef.hidden) {
                return 0;
            }
            if (columnDef.preferredWidth >= 0) {
                return columnDef.preferredWidth;
            }
            const span = columnDef.fillSpan || 1;
            return span * (view.width - view.usedWidth) / view.dynamicColumnCount;
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
                        return this.model.getUrl(index.row);
                });
            }
            model: view.model
        }

        delegate: Item {
            id: item
            required property bool selected
            required property color decoration
            required property var display
            required property var track
            required property string file_url
            required property url cover_art
            required property int row

            implicitHeight: 30

            Loader {
                id: loader
                anchors.fill: parent
                property bool selected: item.selected
                property color decoration: item.decoration
                property var display: item.display
                property var track: item.track
                property url file_url: item.file_url
                property url cover_art: item.cover_art
                property int row: item.row
                property var tableView: view
                property var capabilities: root.model ? root.model.getCapabilities() : Mixxx.LibraryTrackListModel.Capability.None
                sourceComponent: delegate
                focus: true

                onLoaded: {
                // Workaround needed for WaveformOverview column to load the data
                //     if (track)
                //         Mixxx.Library.analyze(track)
                }
            }
            // Workaround needed for WaveformOverview column to load the data
            // TableView.onReused: {
            //     if (track)
            //         Mixxx.Library.analyze(track)
            // }
        }
    }
}
