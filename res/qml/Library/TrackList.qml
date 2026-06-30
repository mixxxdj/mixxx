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

    required property var model
    property var sidebar: model.sidebar()

    property var movedColumn: new Object()

    color: Theme.darkGray

    LibraryComponent.Control {
        id: libraryControl

        onFocusWidgetChanged: {
            switch (focusWidget) {
            case Skin.FocusedWidgetControl.WidgetKind.LibraryView:
                view.forceActiveFocus();
                break;
            }
        }
        onLoadSelectedTrack: (group, play) => {
            view.loadSelectedTrack(group, play);
        }
        onLoadSelectedTrackIntoNextAvailableDeck: play => {
            view.loadSelectedTrackIntoNextAvailableDeck(play);
        }
        onMoveVertical: offset => {
            view.selectionModel.moveSelectionVertical(offset);
        }
    }
    Menu {
        id: columnSelectionMenu

        Instantiator {
            model: root.model.defaultColumns

            delegate: Action {
                property var data: view.getColumn(index)

                checkable: true
                checked: (!view.columnShouldAutoHide(data) || data?.display == Mixxx.TrackListColumn.Display.Show) && data?.display != Mixxx.TrackListColumn.Display.Hide
                text: data?.label ? qsTr(data?.label) : ""

                onTriggered: {
                    // console.log(`columnShouldAutoHide: ${view.columnShouldAutoHide(data)}, want: ${checked}`)
                    if (!view.columnShouldAutoHide(data) === checked){
                        data.display = Mixxx.TrackListColumn.Display.Auto
                    } else {
                        data.display = checked ? Mixxx.TrackListColumn.Display.Show : Mixxx.TrackListColumn.Display.Hide
                    }
                    // console.log(`columnShouldAutoHide: ${data.display}`)
                    let currentExplicitWidth = view.explicitColumnWidth(index)
                    if (data.display == Mixxx.TrackListColumn.Display.Hide) {
                        // console.log(`if ${currentExplicitWidth}`)
                        if (currentExplicitWidth > 0) {
                            data.preferredWidth = currentExplicitWidth;
                        }
                        view.setColumnWidth(index, 0)
                    } else if (data.display != Mixxx.TrackListColumn.Display.Hide) {
                        // console.log(`else ${data.preferredWidth}`)
                        view.setColumnWidth(index, data.preferredWidth)
                    }
                }
            }

            onObjectAdded: (index, object) => columnSelectionMenu.insertAction(index, object)
            onObjectRemoved: (index, object) => columnSelectionMenu.removeAction(object)
        }

        Connections {
            function onColumnMoved(logicalId, oldId, newId) {
                let previousName = columnSelectionMenu.actionAt(logicalId).data.label
                columnSelectionMenu.actionAt(newId).data = view.model.columns[logicalId]
                // console.log(`FFFFFFFFF ${logicalId}: ${previousName} -> ${columnSelectionMenu.actionAt(newId).data.label}`)
            }

            target: view
        }
    }
    HorizontalHeaderView {
        id: horizontalHeader

        property int sortingColumn: -1
        property var sortingOrder: Qt.Descending

        anchors.left: parent.left
        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        syncView: view
        movableColumns: true
        flickableDirection: Flickable.VerticalFlick
        clip: true

        delegate: Item {
            id: column

            required property string display
            required property int index

            implicitHeight: columnName.contentHeight + 5
            implicitWidth: columnName.contentWidth + 5

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (horizontalHeader.sortingColumn == index) {
                        horizontalHeader.sortingOrder = horizontalHeader.sortingOrder == Qt.DescendingOrder ? Qt.AscendingOrder : Qt.DescendingOrder;
                    } else {
                        horizontalHeader.sortingColumn = index;
                        horizontalHeader.sortingOrder = Qt.AscendingOrder;
                    }
                    view.model.sort(horizontalHeader.sortingColumn, horizontalHeader.sortingOrder);
                }
                onLongPressed: (eventPoint, button) => {
                    columnSelectionMenu.popup();
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    columnSelectionMenu.popup();
                }
            }
            Text {
                id: columnName

                anchors.fill: parent
                anchors.leftMargin: 15
                color: Theme.textColor
                elide: Text.ElideRight
                font.capitalization: Font.Capitalize
                font.family: Theme.fontFamily
                font.pixelSize: 12
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignLeft
                text: display
                verticalAlignment: Text.AlignVCenter
            }
            Item {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin: 8
                    top: parent.top
                }
                Label {
                    id: sortIndicator

                    anchors.centerIn: parent
                    color: "red"
                    elide: Text.ElideRight
                    font.bold: true
                    font.capitalization: Font.AllUppercase
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.buttonFontPixelSize
                    horizontalAlignment: Text.AlignRight
                    rotation: horizontalHeader.sortingOrder == Qt.AscendingOrder ? 90 : -90
                    text: "▶"
                    verticalAlignment: Text.AlignVCenter
                    visible: horizontalHeader.sortingColumn == index
                }
            }
            Rectangle {
                id: columnResizer

                color: Theme.darkGray2
                width: 1

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    top: parent.top
                }
                MouseArea {
                    id: columnResizeHandler

                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    preventStealing: true

                    onMouseXChanged: {
                        if (drag.active) {
                            column.width += mouseX;
                            view.setColumnWidth(index, column.width)
                        }
                    }

                    drag {
                        axis: Drag.XAxis
                        target: parent
                        threshold: 2
                    }
                }
            }
        }
    }
    TableView {
        id: view

        onColumnMoved: (logicalIndex, oldVisualIndex, newVisualIndex) => {
            if (root.movedColumn[newVisualIndex] !== undefined && logicalIndex === newVisualIndex){
                delete root.movedColumn[newVisualIndex]
                return;
            }
            console.log(`onColumnMoved: ${logicalIndex} -> ${newVisualIndex}`)
            root.movedColumn[newVisualIndex] = logicalIndex
        }

        function getColumn(index){
            return model.columns[root.movedColumn[index] !== undefined ? root.movedColumn[index] : index]
        }

        function columnShouldAutoHide(column){
            // console.log(`columnShouldAutoHide: ${column.label} ${column.display} ${column.autoHideWidth > 0 && column.autoHideWidth > root.width}`)
            return column?.display == Mixxx.TrackListColumn.Display.Auto && column?.autoHideWidth > 0 && column?.autoHideWidth > root.width
        }

        function loadSelectedTrack(group, play) {
            const urls = this.selectionModel.selectedTrackUrls();
            if (urls.length == 0)
                return;

            Mixxx.PlayerManager.getPlayer(group).loadTrackFromLocationUrl(urls[0], play);
        }
        function loadSelectedTrackIntoNextAvailableDeck(play) {
            const urls = this.selectionModel.selectedTrackUrls();
            if (urls.length == 0)
                return;

            Mixxx.PlayerManager.loadLocationUrlIntoNextAvailableDeck(urls[0], play);
        }

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: horizontalHeader.bottom
        flickableDirection: Flickable.VerticalFlick
        clip: true
        rowHeightProvider: () => 30
        columnWidthProvider: function (column) {
            const columnDef = getColumn(column);
            let explicitWidth = explicitColumnWidth(column);
            if (explicitWidth == -1){
                explicitWidth = columnDef.preferredWidth
            }
            // console.warn(`Column size for ${columnDef.label}: ${explicitWidth} ${view.columnShouldAutoHide(columnDef)} ${root.width}`)
            if (columnDef.display == Mixxx.TrackListColumn.Display.Hide || view.columnShouldAutoHide(columnDef)) {
                return 0;
            }
            if (explicitWidth != -1) {
                return explicitWidth;
            }
            if (model == null) {
                return 0;
            }

            let usedWidth = 0;
            let dynamicColumnCount = 0;
            for (let c = 0; c < model.columns.length; c++) {
                 const columnDef = getColumn(c);
                if (columnDef.display == Mixxx.TrackListColumn.Display.Hide || view.columnShouldAutoHide(columnDef)) {
                    continue;
                }
                let explicitWidth = explicitColumnWidth(c);
                if (explicitWidth == -1){
                    explicitWidth = columnDef.preferredWidth
                }

                if (explicitWidth >= 0) {
                    usedWidth += explicitWidth;
                } else if (explicitWidth != 0) {
                    dynamicColumnCount += columnDef.fillSpan || 1;
                }
            }
            const span = columnDef.fillSpan || 1;
            return span * (view.width - usedWidth) / dynamicColumnCount;
        }
        keyNavigationEnabled: false
        model: root.sidebar.tracklist
        pointerNavigationEnabled: false
        reuseItems: true
        selectionBehavior: TableView.SelectRows

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
        }
        delegate: DelegateChooser {
            role: "columnType"

            DelegateChoice {
                roleValue: Mixxx.TrackListColumn.ColumnType.AlbumArt
                Rectangle {
                    required property url cover_art
                    required property color decoration

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
            }

            DelegateChoice {
                roleValue: Mixxx.TrackListColumn.ColumnType.Overview

                Mixxx.WaveformOverview {
                    id: waveformOverview
                    channels: Mixxx.WaveformOverview.Channels.LeftChannel
                    colorHigh: Theme.white
                    colorLow: Theme.green
                    colorMid: Theme.blue
                    renderer: Mixxx.WaveformOverview.Renderer.Filtered
                    track: null // Lazy loaded
                    implicitHeight: 30
                    implicitWidth: 30

                    Component.onCompleted: {
                        waveformOverview.track = TableView.view.model?.getTrackByRow(row)
                    }
                }
            }
            DelegateChoice {
                Cell {
                    capabilities: root.sidebar.tracklist ? root.sidebar.tracklist.getCapabilities() : Mixxx.LibraryTrackListModel.Capability.None
                    // required property bool selected

                    // readonly property alias dragImage: dragImageSource

                    // Drag.dragType: Drag.Automatic
                    // Drag.mimeData: {
                    //     "text/uri-list": file_url.toString(),
                    //     "text/plain": file_url.toString()
                    // }
                    // Drag.supportedActions: Qt.CopyAction
                    // anchors.fill: parent
                    // color: selected ? Theme.accentColor : (row % 2 == 0 ? Theme.sunkenBackgroundColor : Theme.backgroundColor)

                    Text {
                        // required property string display
                        // id: value

                        anchors.fill: parent
                        // anchors.leftMargin: 15
                        color: Theme.textColor
                        elide: Text.ElideRight
                        font.pixelSize: 14
                        text: display ?? ""
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                // LibraryComponent.TableTextDelegate {}
            }
        }
        selectionModel: ItemSelectionModel {
            function moveSelectionVertical(value) {
                if (value == 0)
                    return;

                const selected = this.selectedIndexes;
                const oldRow = (selected.length == 0) ? 0 : selected[0].row;
                this.selectRow(oldRow + value);
            }
            function selectRow(row) {
                const rowCount = this.model.rowCount();
                if (rowCount == 0) {
                    this.clear();
                    return;
                }
                const newRow = Mixxx.MathUtils.positiveModulo(row, rowCount);
                this.select(this.model.index(newRow, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select | ItemSelectionModel.Clear | ItemSelectionModel.Current);
                if (!view.isRowLoaded(newRow)) {
                    view.positionViewAtRow(newRow, TableView.Visible | TableView.AlignVCenter);
                }
            }
            function selectedTrackUrls() {
                return this.selectedIndexes.map(index => {
                    return this.model.getUrl(index.row);
                });
            }

            model: view.model
        }

        Keys.onDownPressed: this.selectionModel.moveSelectionVertical(1)
        Keys.onEnterPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
        Keys.onReturnPressed: this.loadSelectedTrackIntoNextAvailableDeck(false)
        Keys.onUpPressed: this.selectionModel.moveSelectionVertical(-1)
    }
}
